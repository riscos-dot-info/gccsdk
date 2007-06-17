/* som_runcom.c
 *
 * Copyright 2007 GCCSDK Developers
 * Written by Lee Noar
 */

#include <string.h>
#include <kernel.h>
#include "som_alloc.h"
#include "som_os_swis.h"
#include "som_utilswis.h"
#include "som_elf.h"
#include "som.h"
#include "som_register.h"
#include "som_startapp.h"

/* In order to minimise the amount of SVC stack used at any one time, most of the static
 * functions are marked as not being inlined, otherwise GCC is quite keen on inlining.
 */
#define noinline __attribute__((noinline))

/* Allow 16KB of stack space for program initialisation. This is mostly for the
 * benefit of the Dynamic Loader.
 */
#define SOM_RUN_STACK_SIZE		(16 * 1024)		/* 16KB */

static const char loader_path[] = "DSOLib:lib.";
static const char env_array_name[] = "LD$Env";

typedef struct runcom_state
{
  /* Program name and args extracted from the command line. */
  char *		elf_prog_name;
  char *		elf_prog_args;

  /* Hold information about executable including program headers. */
  elf_file		elf_prog;

  /* Base address of Dynamic Loader. */
  som_PTR		elf_loader_base;

  /* Shared Object Manager handle of Dynamic Loader. */
  som_handle		elf_loader_handle;

  /* Details of environment as returned by OS_GetEnv. */
  os_env_block		env;

  /* Flag indicating whether DDEUtils module is present. */
  bool			ddeutils_is_present;

  /* Flag indicating whether the client was registered so that if
   * an error occurs, we know whether to deregister it.
   */
  bool			client_is_registered;

  /* Pointer to where free unused application memory starts. This is
   * increased as memory is allocated and is also used to ensure that
   * we don't run out of memory due to a small wimpslot.
   */
  som_PTR		free_memory;

  /* The auxillary data array includes an entry that allows the free memory
   * pointer to be passed to the Dynamic Loader, however, when generating the
   * auxillary array, we haven't finished allocating memory yet so this entry can't
   * be filled. This member records the address of the entry so that it can
   * be filled in later.
   */
  unsigned int *	aux_free_mem_slot;

} runcom_state;

static void runcom_init_state(runcom_state *state)
{
  state->elf_prog_name = NULL;
  state->elf_prog_args = NULL;
  elffile_init(&state->elf_prog);

  state->elf_loader_base = NULL;

  state->ddeutils_is_present = ddeutils_is_present();
  state->client_is_registered = false;

  state->free_memory = NULL;

  state->aux_free_mem_slot = NULL;
}

static noinline _kernel_oserror *register_dynamic_loader(runcom_state *state)
{
som_objinfo objinfo;
_kernel_oserror *err = NULL;

  if (state->elf_loader_handle != 0)
  {
    if ((err = som_query_object(state->elf_loader_handle, &objinfo, flag_QUERY_GLOBAL_LIST)) != NULL)
      goto error;
  }
  else
  {
  const Elf32_Ehdr *ehdr = (Elf32_Ehdr *)state->elf_loader_base;
  Elf32_Phdr *phdr = (Elf32_Phdr *)(state->elf_loader_base + ehdr->e_phoff);
  int phnum = ehdr->e_phnum;

    objinfo.base_addr = state->elf_loader_base;

    /* Go through the program headers extracting the info we need. */
    while (phnum--)
    {
      if (phdr->p_type == PT_LOAD)
      { /* A writable, loadable segment is a data segment. */
	if ((phdr->p_flags & PF_W) != 0)
	{
	  objinfo.public_rw_ptr = state->elf_loader_base + phdr->p_vaddr;
	  objinfo.rw_size = phdr->p_memsz;
	  objinfo.bss_offset = phdr->p_filesz;
	  objinfo.bss_size = phdr->p_memsz - phdr->p_filesz;
	}
      }
      else if (phdr->p_type == PT_DYNAMIC)
      {
	objinfo.dyn_offset = (phdr->p_vaddr + state->elf_loader_base) - objinfo.public_rw_ptr;
	objinfo.dyn_size = phdr->p_memsz;

	/* Find the GOT of the loader. */
      unsigned int *dt;

	for (dt = (unsigned int *)(state->elf_loader_base + phdr->p_vaddr); *dt != DT_NULL; dt += 2)
	  if (*dt == DT_PLTGOT)
	  {
	    objinfo.got_offset = (*(dt + 1) + state->elf_loader_base) - objinfo.public_rw_ptr;
	    break;
	  }

	if (*dt == DT_NULL)
	{
	  err = somerr_dl_no_got;
	  goto error;
	}
      }

      phdr++;
    }

    /* INTERP_NAME is a pointer to the actual string within the ELF file, so it's safe
     * to use it without worrying about it disappearing. */
    objinfo.name = state->elf_prog.interp_name;

    state->elf_loader_handle = (som_handle)objinfo.base_addr;
  }

  /* Store the client's copy of the dynamic loader's data segment at the previously
   * marked free memory location. */
  objinfo.private_rw_ptr = state->free_memory;

  /* Advance the free memory pointer. */
  state->free_memory = word_align(state->free_memory + objinfo.rw_size);

  objinfo.flags.type = object_flag_type_LOADER;

som_object *object;
  if ((err = som_register_sharedobject(state->elf_loader_handle, &objinfo, &object)) != NULL)
    goto error;

  /* Copy the R/W segment from the public copy to the private copy. */
  memcpy(objinfo.private_rw_ptr, objinfo.public_rw_ptr, objinfo.rw_size);

  /* Zero the bss area. */
  memset(objinfo.private_rw_ptr + objinfo.bss_offset, 0, objinfo.bss_size);

  return NULL;

error:
  return err;
}

static noinline _kernel_oserror *register_client(runcom_state *state)
{
som_objinfo objinfo;

  objinfo.base_addr = 0;

Elf32_Phdr *phdr = state->elf_prog.prog_headers;
int phnum = state->elf_prog.elf_header.e_phnum;

  /* This relies on the DYNAMIC segment following the loadable segments. This always
   * seems to be the case. */
  while (phnum--)
  {
    /* Look for a writable, loadable segment, ie, a data segment. */
    if (phdr->p_type == PT_LOAD)
    {
      if ((phdr->p_flags & PF_W) != 0)
      {
	objinfo.public_rw_ptr = (som_PTR)phdr->p_vaddr;
	objinfo.private_rw_ptr = (som_PTR)phdr->p_vaddr;
	objinfo.rw_size = phdr->p_memsz;
	objinfo.bss_offset = phdr->p_filesz;
	objinfo.bss_size = phdr->p_memsz - phdr->p_filesz;
      }
    }
    else if (phdr->p_type == PT_DYNAMIC)
    {
      objinfo.dyn_offset = (som_PTR)phdr->p_vaddr - objinfo.public_rw_ptr;
      objinfo.dyn_size = phdr->p_memsz;

      /* Find the GOT of the client. */
    unsigned int *dt;

      for (dt = (unsigned int *)phdr->p_vaddr; *dt != DT_NULL; dt += 2)
	if (*dt == DT_PLTGOT)
	{
	  objinfo.got_offset = (som_PTR)*(dt + 1) - objinfo.public_rw_ptr;
	  break;
	}
    }

    phdr++;
  }

  objinfo.name = state->elf_prog_name;

_kernel_oserror *err;

  if ((err = som_register_client(0x8000, &objinfo)) == NULL)
    state->client_is_registered = true;

  return err;
}

static noinline _kernel_oserror *load_dynamic_loader(runcom_state *state)
{
_kernel_oserror *err;
char *filename = NULL;
elf_file loader;

  elffile_init(&loader);

  if ((err = som_alloc(strlen(loader_path) + strlen(state->elf_prog.interp_name) + 1, (void **)(void *)&filename)) != NULL)
    return err;
  strcpy(filename, loader_path);
  strcat(filename, state->elf_prog.interp_name);

  if ((err = elffile_open(filename, &loader)) != NULL)
    goto error;

  /* Don't need the filename anymore. */
  som_free(filename);
  filename = NULL;

  /* Allocate enough memory to store all of the PT_LOAD segments. */
  if ((err = som_alloc_lib(elffile_memory_size(&loader), (void **)(void *)&state->elf_loader_base)) == NULL)
  {
  Elf32_Phdr *phdr = loader.prog_headers;
  int phnum = loader.elf_header.e_phnum;

    /* Load all loadable segments of the Dynamic Loader. */
    while (phnum--)
    {
      if (phdr->p_type == PT_LOAD)
      {
	fseek(loader.handle, phdr->p_offset, SEEK_SET);
	if (fread(phdr->p_vaddr + state->elf_loader_base, phdr->p_filesz, 1, loader.handle) != 1)
	  goto error;
      }

      phdr++;
    }
  }
  else
    goto error;

  elffile_close(&loader);

  return NULL;

error:
  if (!err)
    if ((err = _kernel_last_oserror()) == NULL)
      err = somerr_file_error; /* Default to file error */

  if (filename)
    som_free(filename);

  if (state->elf_loader_base)
  {
    som_free(state->elf_loader_base);
    state->elf_loader_base = NULL;
  }

  elffile_close(&loader);

  return err;
}

/* argv consists of only the program name for now as that's all the Dynamic Loader
 * requires, but it would be easy to include all args as well. */
static noinline _kernel_oserror *setup_argv_array(runcom_state *state)
{
#define NUM_ARGV_ARRAY_ENTRIES 3 /* argc + program name + terminating NULL entry. */
char *argv_data = (char *)state->free_memory;

  /* Make sure there's enough space for the program name. */
  if (state->free_memory + strlen(state->elf_prog_name) + 1 >= (som_PTR)state->env.ram_limit)
    return somerr_wimpslot_too_small;
  state->free_memory = word_align(state->free_memory + strlen(state->elf_prog_name) + 1);

char **argv_array = (char **)(state->env.ram_limit - 4 * NUM_ARGV_ARRAY_ENTRIES);

  /* Make sure there's enough space for the array. */
  if (state->free_memory >= (som_PTR)argv_array)
    return somerr_wimpslot_too_small;

  strcpy(argv_data, state->elf_prog_name);

  argv_array[0] = (char *)1; /* argc */
  argv_array[1] = argv_data;
  argv_array[2] = NULL;

  state->env.ram_limit = (unsigned int)argv_array;

  return NULL;
}

/* The aux array is written backwards, in the reverse order that the
 * Dynamic Loader will read it, from the current top of memory working downwards.
 */
static noinline _kernel_oserror *setup_aux_array(runcom_state *state)
{
#define NUM_AUX_ARRAY_ENTRIES 6
unsigned int *array = (unsigned int *)state->env.ram_limit;

  /* Make sure there's enough space for the auxillary array. */
  if ((som_PTR)(array - (NUM_AUX_ARRAY_ENTRIES * 2)) <= state->free_memory)
    return somerr_wimpslot_too_small;

  /* Null terminate array. */
  *--array = 0;
  *--array = AT_NULL;

  /* Tell Dynamic Loader where free memory starts. */
  /* We don't know where free memory will eventually start for the Dynamic Loader as
   * we haven't finished using it yet, so record the address of the slot where it should
   * be stored for later.
   */
  *--array = 0;
  state->aux_free_mem_slot = (unsigned int *)array;
  *--array = AT_DATA;

  /* Store entry point of ELF program. */
  *--array = state->elf_prog.elf_header.e_entry;
  *--array = AT_ENTRY;

  /* Store base address of dynamic loader. */
  *--array = (unsigned int)state->elf_loader_base;
  *--array = AT_BASE;

  /* Store the number of program headers in the ELF program. */
  *--array = state->elf_prog.elf_header.e_phnum;
  *--array = AT_PHNUM;

  /* Store the ELF program's program header address. */
  *--array = 0x8000 + state->elf_prog.elf_header.e_phoff;
  *--array = AT_PHDR;

  /* Keep track of the data we're storing on the stack. */
  state->env.ram_limit = (unsigned int)array;

  return NULL;
}

static noinline _kernel_oserror *setup_env_array(runcom_state *state)
{
int env_size = os_read_var_val_size(env_array_name);

  /* Make sure we have enough space to store the string. */
  if (state->free_memory + env_size > (som_PTR)state->env.ram_limit)
    return somerr_wimpslot_too_small;

char *env_string = (char *)state->free_memory;

  os_read_var_val(env_array_name, env_string, env_size);

  state->free_memory = word_align(state->free_memory + env_size);

  /* Count number of variables in environment.
   * Number of variables = number of spaces + 1.
   * Note this doesn't work for an empty string.
   */
int count = 1; /* Start with count of 1 for terminator. */

  if (*env_string != '\0')
  {
  char *p;

    count++;
    for (p = env_string; *p != '\0'; p++)
      if (*p == ' ')
	count++;
  }

  /* Size of env array in bytes is number of variables * 4.
   * Make sure we have the space for it. */
  if (state->free_memory > (som_PTR)state->env.ram_limit - (count << 2))
    return somerr_wimpslot_too_small;

char **env_array = (char **)(state->env.ram_limit - (count << 2));

  /* Keep track of the data we're storing on the stack. */
  state->env.ram_limit = (unsigned int)env_array;

  if (*env_string != '\0')
  {
  char *p;

    *env_array++ = env_string;
    for (p = env_string; *p != '\0'; p++)
      if (*p == ' ')
      {
	*p++ = '\0';
	*env_array++ = p;
      }
  }

  /* Terminate array. */
  *env_array = NULL;

  return NULL;
}

_kernel_oserror *command_run(const char *arg_string, int argc)
{
_kernel_oserror *err;
runcom_state *state = NULL;
bool dl_already_loaded = false;

  if ((err = som_alloc(sizeof(runcom_state), (void **)(void *)&state)) != NULL)
    return err;

  runcom_init_state(state);

  /* Make a copy of the command line, taking the DDEUtils CL into account. */
int dde_cl_len = ddeutils_get_cl_size();

  /* Allow for space and terminator. */
  if ((err = som_alloc(dde_cl_len + strlen(arg_string) + 2, (void **)(void *)&state->elf_prog_name)) != NULL)
    goto error;

  strcpy(state->elf_prog_name, arg_string);

  if (dde_cl_len > 0)
  {
    strcat(state->elf_prog_name, " ");
    ddeutils_get_cl(state->elf_prog_name + strlen(state->elf_prog_name));
  }

  state->elf_prog_args = strchr(state->elf_prog_name, ' ');

  if (state->elf_prog_args)
    *state->elf_prog_args++ = '\0';

  if ((err = elffile_open(state->elf_prog_name, &state->elf_prog)) != NULL)
    goto error;
  /* Don't close the file until we're sure that we've finished with all info in the
   * elf_file structure, eg, program headers. */

  /* If the DDEUtils module is loaded, then we use that for the arguments and just pass the ELF
   * file name to the OS. */
  if ((err = os_start_app(state->ddeutils_is_present ? "" : state->elf_prog_args,
			  elffile_entry_point(&state->elf_prog),
			  state->elf_prog_name)) != NULL)
    goto error;

  os_get_env(&state->env);

  if ((err = elffile_load(&state->elf_prog, state->env.ram_limit - SOM_RUN_STACK_SIZE, &state->free_memory)) != NULL)
    goto error;

  /* If the DDEUtils module is loaded, then pass the arguments via the DDEUtils
   * command line. If it's not loaded, then do nothing as OS_FSControl,2 has already
   * setup the OS environment for both command and args.
   */
  if (state->ddeutils_is_present)
    ddeutils_set_cl(state->elf_prog_args);

  if (state->elf_prog.dynamic_seg == NULL)
  {
    /* No dynamic segment, so assume statically linked. */
  som_PTR entry_point = (som_PTR)state->elf_prog.elf_header.e_entry;
  unsigned int ram_limit = state->env.ram_limit;

    elffile_close(&state->elf_prog);

    /* Finished with command line. */
    som_free(state->elf_prog_name);
    state->elf_prog_name = NULL;

    som_free(state);

    som_start_app(entry_point, ram_limit, SOM_RUN_STACK_SIZE);
  }
  else
  {
    /* Check if the required loader is already loaded by the system. */
    state->elf_loader_handle = som_handle_from_name(state->elf_prog.interp_name);

    /* A zero handle means the loader isn't known to the system yet. */
    if (state->elf_loader_handle == 0)
    {
      /* The loader isn't in the system yet, so we have to load it first. */
      dl_already_loaded = false;
      err = load_dynamic_loader(state);
    }
    else
    {
    som_objinfo objinfo;

      /* The loader is already present in the system, we just need the address that
       * it's loaded at. */
      dl_already_loaded = true;
      err = som_query_object(state->elf_loader_handle, &objinfo, flag_QUERY_GLOBAL_LIST);
      state->elf_loader_base = objinfo.base_addr;
    }

    if (err)
      goto error;

    /* Register the client here now that we know the Dynamic Loader is present */
    if ((err = register_client(state)) != NULL)
      goto error;

    if ((err = register_dynamic_loader(state)) != NULL)
      goto error;

    if ((err = som_generate_got_array()) != NULL)
      goto error;

    /* The order that these are called in is important to ensure the arrays are constructed in the
     * order the Dynamic Loader expects to find them. */
    if ((err = setup_aux_array(state)) != NULL ||
	(err = setup_env_array(state)) != NULL ||
	(err = setup_argv_array(state)) != NULL)
      goto error;

    elffile_close(&state->elf_prog);

    /* Now that we've finished allocating from the free memory pool, we can fill in the auxillary
     * data array with a pointer to where it starts.
     */
    if (state->aux_free_mem_slot)
      *state->aux_free_mem_slot = (unsigned int)state->free_memory;

    /* Finished with command line. */
    som_free(state->elf_prog_name);
    state->elf_prog_name = NULL;

  const Elf32_Ehdr *ehdr = (Elf32_Ehdr *)state->elf_loader_base;
  unsigned int ram_limit = state->env.ram_limit;

    som_free(state);

    som_start_app((som_PTR)ehdr + ehdr->e_entry, ram_limit, SOM_RUN_STACK_SIZE);
  }

  /* Should never get to here. */

  return NULL;

error:
  if (state)
  {
    /* It's possible that an error may occur after the Dynamic Loader has loaded, but
     * before it has properly registered. If this is the first client to use the DL and
     * therefore load it, then the memory used to store the DL needs to be freed or else
     * it will be lost. Note that the module finalisation routine will not free it as the
     * DL is not linked into the list.
     */
    if (dl_already_loaded == false && state->elf_loader_base != NULL)
      som_free(state->elf_loader_base);

    if (state->client_is_registered)
      som_deregister_client();

    if (state->elf_prog_name)
      som_free(state->elf_prog_name);

    elffile_close(&state->elf_prog);

    som_free(state);
  }

  return err;
}

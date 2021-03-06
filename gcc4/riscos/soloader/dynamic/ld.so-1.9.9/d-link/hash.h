#include "link.h"

#ifndef RTLD_NEXT
#define RTLD_NEXT	((void*)-1)
#endif

struct dyn_elf{
  unsigned int flags;
  struct elf_resolve * dyn;
  struct dyn_elf * next_handle;  /* Used by dlopen et al. */
  struct dyn_elf * next;
};

struct elf_resolve{
  /* These entries must be in this order to be compatible with the interface used
     by gdb to obtain the list of symbols. */
  char * loadaddr;
  char * libname;
  unsigned int dynamic_addr;
  struct elf_resolve * next;
  struct elf_resolve * prev;
  /* Nothing after this address is used by gdb. */
  enum {elf_lib, elf_executable,program_interpreter, loaded_file} libtype;
  struct dyn_elf * symbol_scope;
  unsigned short usage_count;
  unsigned short int init_flag;
  unsigned int nbucket;
  unsigned int * elf_buckets;
  /*
   * These are only used with ELF style shared libraries
   */
  unsigned int nchain;
  unsigned int * chains;
  unsigned int dynamic_info[29];

  unsigned int dynamic_size;
  unsigned int n_phent;
  Elf32_Phdr * ppnt;

  /* End of text segment.  */
  char *endaddr;

  /* Address of ARM stack unwinder exception tables.  */
  _Unwind_Ptr exidx;

  /* Size, in bytes, of exception tables.  */
  int exidx_size;

  /* Pointer to a string in the image that gives the version of the
     shared library ABI.  */
  char *abi_version;

  /* Pointer to the client R/W data. Used by dlclose() to free the memory.  */
  void *rw_addr;

  /* The ELF flags copied from the ELF image.  */
  unsigned elf_flags;
};

#if 0
/*
 * The DT_DEBUG entry in the .dynamic section is given the address of this structure.
 * gdb can pick this up to obtain the correct list of loaded modules.
 */

struct r_debug{
  int r_version;
  struct elf_resolve * link_map;
  unsigned long brk_fun;
  enum {RT_CONSISTENT, RT_ADD, RT_DELETE};
  unsigned long ldbase;
};
#endif

#define COPY_RELOCS_DONE 1
#define RELOCS_DONE 2
#define JMP_RELOCS_DONE 4
#define INIT_FUNCS_CALLED 8

extern struct dyn_elf     * _dl_symbol_tables;
extern struct elf_resolve * _dl_loaded_modules;
extern struct dyn_elf 	  * _dl_handles;

extern struct elf_resolve * _dl_check_hashed_files(char * libname);
extern struct elf_resolve * _dl_add_elf_hash_table(char * libname,
					       char * loadaddr,
					       unsigned int * dynamic_info,
					       unsigned int dynamic_addr,
					       unsigned int dynamic_size);
extern char * _dl_find_hash(char * name, struct dyn_elf * rpnt1,
			    struct elf_resolve **lib,
			    struct elf_resolve * f_tpnt,
			    int copyrel);

extern int _dl_linux_dynamic_link(void);

#ifdef __mc68000__
/* On m68k constant strings are referenced through the GOT. */
/* XXX Requires load_addr to be defined. */
#define SEND_STDERR(X)				\
  { const char *__s = (X);			\
    if (__s < (const char *) load_addr) __s += load_addr;	\
    _dl_write (2, __s, _dl_strlen (__s));	\
  }
#else
#define SEND_STDERR(X) _dl_write(2, X, _dl_strlen(X));
#endif

extern int _dl_fdprintf(int, const char *, ...);
extern char * _dl_library_path;
extern char * _dl_strdup(const char *);
unsigned long _dl_elf_hash(const char * name);

static inline int _dl_symbol(char * name)
{
  if(name[0] != '_' || name[1] != 'd' || name[2] != 'l' || name[3] != '_')
    return 0;
  return 1;
}

#define DL_ERROR_NOFILE 1
#define DL_ERROR_NOZERO 2
#define DL_ERROR_NOTELF 3
#define DL_ERROR_NOTMAGIC 4
#define DL_ERROR_NOTDYN 5
#define DL_ERROR_MMAP_FAILED 6
#define DL_ERROR_NODYNAMIC 7
#define DL_WRONG_RELOCS 8
#define DL_BAD_HANDLE 9
#define DL_NO_SYMBOL 10


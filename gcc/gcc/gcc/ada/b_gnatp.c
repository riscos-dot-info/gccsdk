#ifdef __STDC__
#define PARAMS(paramlist) paramlist
#else
#define PARAMS(paramlist) ()
#endif

extern void __gnat_set_globals 
 PARAMS ((int, int, int, int, int, int, 
          void (*) PARAMS ((void)), int, int));
extern void adafinal PARAMS ((void));
extern void adainit PARAMS ((void));
extern void system__standard_library__adafinal PARAMS ((void));
extern int main PARAMS ((int, char **, char **));
extern void exit PARAMS ((int));
extern void __gnat_break_start PARAMS ((void));
extern void _ada_gnatprep PARAMS ((void));
extern void __gnat_initialize PARAMS ((void));
extern void __gnat_finalize PARAMS ((void));
extern void __gnat_install_handler PARAMS ((void));

extern void interfaces__c_streams___elabs PARAMS ((void));
extern void ada__exceptions___elabs PARAMS ((void));
extern void system__exception_table___elabb PARAMS ((void));
extern void ada__io_exceptions___elabs PARAMS ((void));
extern void ada__strings___elabs PARAMS ((void));
extern void system__exceptions___elabs PARAMS ((void));
extern void ada__tags___elabs PARAMS ((void));
extern void ada__tags___elabb PARAMS ((void));
extern void ada__streams___elabs PARAMS ((void));
extern void system__soft_links___elabb PARAMS ((void));
extern void system__secondary_stack___elabb PARAMS ((void));
extern void system__finalization_root___elabs PARAMS ((void));
extern void ada__exceptions___elabb PARAMS ((void));
extern void ada__strings__maps___elabs PARAMS ((void));
extern void ada__strings__maps__constants___elabs PARAMS ((void));
extern void system__finalization_implementation___elabs PARAMS ((void));
extern void system__finalization_implementation___elabb PARAMS ((void));
extern void ada__finalization___elabs PARAMS ((void));
extern void ada__finalization__list_controller___elabs PARAMS ((void));
extern void gnat__directory_operations___elabs PARAMS ((void));
extern void gnat__regexp___elabs PARAMS ((void));
extern void gnat__command_line___elabs PARAMS ((void));
extern void gnat__command_line___elabb PARAMS ((void));
extern void system__file_control_block___elabs PARAMS ((void));
extern void system__file_io___elabb PARAMS ((void));
extern void ada__text_io___elabs PARAMS ((void));
extern void ada__text_io___elabb PARAMS ((void));

extern int  __gnat_handler_installed;

extern int gnat_argc;
extern char **gnat_argv;
extern char **gnat_envp;
extern int gnat_exit_status;


char __gnat_version[] = "GNAT Version: 5.00w (20010924)";
char __gnat_ada_main_program_name[] = "_ada_gnatprep";
void adafinal () {
   system__standard_library__adafinal ();
}

void adainit ()
{
   extern char interfaces__c_streams_E;
   extern char ada__exceptions_E;
   extern char system__exception_table_E;
   extern char ada__io_exceptions_E;
   extern char ada__strings_E;
   extern char system__exceptions_E;
   extern char system__secondary_stack_E;
   extern char ada__tags_E;
   extern char ada__streams_E;
   extern char system__soft_links_E;
   extern char system__finalization_root_E;
   extern char ada__strings__maps_E;
   extern char ada__strings__maps__constants_E;
   extern char system__finalization_implementation_E;
   extern char ada__finalization_E;
   extern char ada__finalization__list_controller_E;
   extern char gnat__directory_operations_E;
   extern char gnat__regexp_E;
   extern char gnat__command_line_E;
   extern char system__file_control_block_E;
   extern char system__file_io_E;
   extern char ada__text_io_E;
   extern char gnatprep_E;

   __gnat_set_globals (
      -1,      /* Main_Priority              */
      -1,      /* Time_Slice_Value           */
      'b',     /* WC_Encoding                */
      ' ',     /* Locking_Policy             */
      ' ',     /* Queuing_Policy             */
      ' ',     /* Tasking_Dispatching_Policy */
      0,       /* Finalization routine address, not used anymore */
      0,       /* Unreserve_All_Interrupts */
      0);      /* Exception_Tracebacks */
   if (__gnat_handler_installed == 0)
     {
        __gnat_install_handler ();
     }

   if (interfaces__c_streams_E == 0) {
      interfaces__c_streams___elabs ();
   }
   interfaces__c_streams_E = 1;
   if (ada__exceptions_E == 0) {
      ada__exceptions___elabs ();
   }
   if (system__exception_table_E == 0) {
      system__exception_table___elabb ();
      system__exception_table_E++;
   }
   if (ada__io_exceptions_E == 0) {
      ada__io_exceptions___elabs ();
      ada__io_exceptions_E++;
   }
   if (ada__strings_E == 0) {
      ada__strings___elabs ();
      ada__strings_E++;
   }
   if (system__exceptions_E == 0) {
      system__exceptions___elabs ();
      system__exceptions_E++;
   }
   if (ada__tags_E == 0) {
      ada__tags___elabs ();
   }
   if (ada__tags_E == 0) {
      ada__tags___elabb ();
      ada__tags_E++;
   }
   if (ada__streams_E == 0) {
      ada__streams___elabs ();
      ada__streams_E++;
   }
   if (system__soft_links_E == 0) {
      system__soft_links___elabb ();
      system__soft_links_E++;
   }
   if (system__secondary_stack_E == 0) {
      system__secondary_stack___elabb ();
      system__secondary_stack_E++;
   }
   if (system__finalization_root_E == 0) {
      system__finalization_root___elabs ();
   }
   system__finalization_root_E = 1;
   if (ada__exceptions_E == 0) {
      ada__exceptions___elabb ();
      ada__exceptions_E++;
   }
   if (ada__strings__maps_E == 0) {
      ada__strings__maps___elabs ();
   }
   ada__strings__maps_E = 1;
   if (ada__strings__maps__constants_E == 0) {
      ada__strings__maps__constants___elabs ();
      ada__strings__maps__constants_E++;
   }
   if (system__finalization_implementation_E == 0) {
      system__finalization_implementation___elabs ();
   }
   if (system__finalization_implementation_E == 0) {
      system__finalization_implementation___elabb ();
      system__finalization_implementation_E++;
   }
   if (ada__finalization_E == 0) {
      ada__finalization___elabs ();
   }
   ada__finalization_E = 1;
   if (ada__finalization__list_controller_E == 0) {
      ada__finalization__list_controller___elabs ();
   }
   ada__finalization__list_controller_E = 1;
   if (gnat__directory_operations_E == 0) {
      gnat__directory_operations___elabs ();
   }
   gnat__directory_operations_E = 1;
   if (gnat__regexp_E == 0) {
      gnat__regexp___elabs ();
   }
   gnat__regexp_E = 1;
   if (gnat__command_line_E == 0) {
      gnat__command_line___elabs ();
   }
   if (gnat__command_line_E == 0) {
      gnat__command_line___elabb ();
      gnat__command_line_E++;
   }
   if (system__file_control_block_E == 0) {
      system__file_control_block___elabs ();
      system__file_control_block_E++;
   }
   if (system__file_io_E == 0) {
      system__file_io___elabb ();
      system__file_io_E++;
   }
   if (ada__text_io_E == 0) {
      ada__text_io___elabs ();
   }
   if (ada__text_io_E == 0) {
      ada__text_io___elabb ();
      ada__text_io_E++;
   }
   gnatprep_E = 1;
}
int main (argc, argv, envp)
    int argc;
    char **argv;
    char **envp;
{
   gnat_argc = argc;
   gnat_argv = argv;
   gnat_envp = envp;
 
   __gnat_initialize ();
   adainit ();
   __gnat_break_start ();
 
   _ada_gnatprep ();
 
   system__standard_library__adafinal ();
   __gnat_finalize ();
   exit (gnat_exit_status);
}
unsigned gnatprepB = 0xf759e8e9;
unsigned gnatprepS = 0x466cfba4;
unsigned system__standard_libraryB = 0xc0e50ac3;
unsigned system__standard_libraryS = 0x6d0b25e7;
unsigned adaS = 0xa0108083;
unsigned ada__charactersS = 0x060f352a;
unsigned ada__characters__handlingB = 0xf6447ab9;
unsigned ada__characters__handlingS = 0x46d230ec;
unsigned ada__characters__latin_1S = 0xa0be30a8;
unsigned ada__stringsS = 0xc4504387;
unsigned systemS = 0x5f7c428f;
unsigned system__exception_tableB = 0x58c090c8;
unsigned system__exception_tableS = 0x7bff8c6d;
unsigned gnatS = 0x6380ea48;
unsigned gnat__htableB = 0x2c01a59d;
unsigned gnat__htableS = 0xeb251ef2;
unsigned ada__strings__mapsB = 0x7f70d6fb;
unsigned ada__strings__mapsS = 0x841ddc3f;
unsigned ada__exceptionsB = 0x98bed957;
unsigned ada__exceptionsS = 0x20d65858;
unsigned gnat__heap_sort_aB = 0xc5fbb2c4;
unsigned gnat__heap_sort_aS = 0xdc301c49;
unsigned system__exceptionsS = 0xbbfe2e15;
unsigned system__machine_state_operationsB = 0xafcd5e0c;
unsigned system__machine_state_operationsS = 0x65adb8f1;
unsigned system__storage_elementsB = 0xa4e49d90;
unsigned system__storage_elementsS = 0x99c2b222;
unsigned system__secondary_stackB = 0xdedafdd0;
unsigned system__secondary_stackS = 0x7a466d01;
unsigned system__parametersB = 0x38b39edd;
unsigned system__parametersS = 0xb77e93cc;
unsigned system__soft_linksB = 0xfd0fec16;
unsigned system__soft_linksS = 0xccb8d065;
unsigned system__stack_checkingB = 0x31fdd957;
unsigned system__stack_checkingS = 0x16073c88;
unsigned system__tracebackB = 0x4d4e8ee2;
unsigned system__tracebackS = 0x69317710;
unsigned system__bit_opsB = 0xb73aa381;
unsigned system__bit_opsS = 0x0c033dfd;
unsigned gnat__exceptionsS = 0x350327a4;
unsigned system__unsigned_typesS = 0x5ad175bd;
unsigned ada__strings__maps__constantsS = 0x6b21310c;
unsigned ada__command_lineB = 0x25dfd6a9;
unsigned ada__command_lineS = 0xd8088025;
unsigned ada__strings__fixedB = 0xa757981e;
unsigned ada__strings__fixedS = 0xa09484f4;
unsigned ada__strings__searchB = 0x6b03b68b;
unsigned ada__strings__searchS = 0xebae4dd3;
unsigned ada__text_ioB = 0x53b374d1;
unsigned ada__text_ioS = 0x3631d409;
unsigned ada__streamsS = 0x30f54674;
unsigned ada__tagsB = 0x3c7217eb;
unsigned ada__tagsS = 0x85ab19c8;
unsigned interfacesS = 0x6e859da6;
unsigned interfaces__c_streamsB = 0xcf1b6eec;
unsigned interfaces__c_streamsS = 0x9dcf6698;
unsigned system__file_ioB = 0x5a728517;
unsigned system__file_ioS = 0x5951418b;
unsigned ada__finalizationB = 0x78cccf5d;
unsigned ada__finalizationS = 0x0af0a97f;
unsigned system__finalization_rootB = 0x2707dc21;
unsigned system__finalization_rootS = 0xb1c78303;
unsigned system__finalization_implementationB = 0xf489f239;
unsigned system__finalization_implementationS = 0xce47eeea;
unsigned system__string_ops_concat_3B = 0x9d5e2b8a;
unsigned system__string_ops_concat_3S = 0xa4207beb;
unsigned system__string_opsB = 0xcf0ab27e;
unsigned system__string_opsS = 0x424b2b8f;
unsigned system__stream_attributesB = 0x951bf591;
unsigned system__stream_attributesS = 0x0a977e44;
unsigned ada__io_exceptionsS = 0x753e9209;
unsigned system__file_control_blockS = 0xdbeadbc4;
unsigned ada__finalization__list_controllerB = 0xffad3e68;
unsigned ada__finalization__list_controllerS = 0x29e5afbc;
unsigned gnat__command_lineB = 0x9c9e9b05;
unsigned gnat__command_lineS = 0xabcdab3f;
unsigned gnat__directory_operationsB = 0x34a5934c;
unsigned gnat__directory_operationsS = 0xaef6d19c;
unsigned gnat__os_libB = 0x0c19a86b;
unsigned gnat__os_libS = 0x8c649cc0;
unsigned gnat__regexpB = 0x12200625;
unsigned gnat__regexpS = 0x60cbc153;
unsigned gnat__case_utilB = 0x1e439027;
unsigned gnat__case_utilS = 0xf3d435e7;
unsigned system__img_intB = 0x70f6a786;
unsigned system__img_intS = 0xaec6c6ec;
unsigned gnatvsnS = 0x575b423a;
unsigned system__string_ops_concat_4B = 0xcfb745b6;
unsigned system__string_ops_concat_4S = 0x63b86e29;
unsigned system__memoryB = 0xeac08eae;
unsigned system__memoryS = 0x528e943f;

/* BEGIN ELABORATION ORDER
ada (spec)
ada.characters (spec)
ada.characters.handling (spec)
ada.characters.latin_1 (spec)
ada.command_line (spec)
gnat (spec)
gnat.case_util (spec)
gnat.case_util (body)
gnat.exceptions (spec)
gnat.heap_sort_a (spec)
gnat.heap_sort_a (body)
gnat.htable (spec)
gnat.htable (body)
interfaces (spec)
system (spec)
system.bit_ops (spec)
system.img_int (spec)
system.parameters (spec)
system.parameters (body)
interfaces.c_streams (spec)
interfaces.c_streams (body)
system.standard_library (spec)
ada.exceptions (spec)
system.exception_table (spec)
system.exception_table (body)
ada.io_exceptions (spec)
ada.strings (spec)
system.exceptions (spec)
system.storage_elements (spec)
system.storage_elements (body)
system.machine_state_operations (spec)
system.machine_state_operations (body)
system.secondary_stack (spec)
system.img_int (body)
ada.command_line (body)
ada.tags (spec)
ada.tags (body)
ada.streams (spec)
system.stack_checking (spec)
system.soft_links (spec)
system.soft_links (body)
system.stack_checking (body)
system.secondary_stack (body)
gnat.os_lib (spec)
gnat.os_lib (body)
system.finalization_root (spec)
system.finalization_root (body)
system.memory (spec)
system.memory (body)
system.standard_library (body)
system.string_ops (spec)
system.string_ops (body)
system.string_ops_concat_3 (spec)
system.string_ops_concat_3 (body)
system.string_ops_concat_4 (spec)
system.string_ops_concat_4 (body)
system.traceback (spec)
system.traceback (body)
ada.exceptions (body)
system.unsigned_types (spec)
system.bit_ops (body)
ada.strings.maps (spec)
ada.strings.maps (body)
ada.strings.fixed (spec)
ada.strings.maps.constants (spec)
ada.characters.handling (body)
ada.strings.search (spec)
ada.strings.search (body)
ada.strings.fixed (body)
system.stream_attributes (spec)
system.stream_attributes (body)
system.finalization_implementation (spec)
system.finalization_implementation (body)
ada.finalization (spec)
ada.finalization (body)
ada.finalization.list_controller (spec)
ada.finalization.list_controller (body)
gnat.directory_operations (spec)
gnat.directory_operations (body)
gnat.regexp (spec)
gnat.regexp (body)
gnat.command_line (spec)
gnat.command_line (body)
system.file_control_block (spec)
system.file_io (spec)
system.file_io (body)
ada.text_io (spec)
ada.text_io (body)
gnatprep (spec)
gnatvsn (spec)
gnatprep (body)
   END ELABORATION ORDER */

/* BEGIN Object file/option list
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/ada.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-charac.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-chlat1.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/gnat.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/g-casuti.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/g-except.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/g-hesora.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/g-htable.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/interfac.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/system.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-parame.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/i-cstrea.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-exctab.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-ioexce.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-string.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-except.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-stoele.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-mastop.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-imgint.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-comlin.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-tags.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-stream.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-soflin.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-stache.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-secsta.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/g-os_lib.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-finroo.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-memory.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-stalib.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-strops.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-sopco3.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-sopco4.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-traceb.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-except.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-unstyp.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-bitops.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-strmap.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-stmaco.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-chahan.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-strsea.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-strfix.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-stratt.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-finimp.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-finali.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-filico.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/g-dirope.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/g-regexp.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/g-comlin.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-ficobl.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/s-fileio.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/a-textio.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/gnatvsn.o
/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/gnatprep.o
-L/home/nick/gccsdk-3.0/arm-riscos-aof/gcc-3_1_0/gcc/stage1/ada/
-L/usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.1/adalib/
-static
-lgnat
   END Object file/option list */

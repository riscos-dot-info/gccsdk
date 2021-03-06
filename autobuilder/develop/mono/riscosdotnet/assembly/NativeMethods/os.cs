//
// NativeMethods/os.cs - a binding of the RISC OS API to C#.
//
// Author: Lee Noar (leenoar@sky.com)
//

using System;
using System.Runtime.InteropServices;

namespace riscos
{
	// Structures that need to be laid out so that they match those used
	// by RISC OS and may need to be embedded in other objects.
	// Embedded objects have to be structs as classes are reference objects.
	// Embedding a reference object means we embed a pointer to it rather
	// than the object itself.
	public static class NativeOS
	{
		[StructLayout(LayoutKind.Sequential)]
		public struct Rect
		{
			public int MinX;
			public int MinY;
			public int MaxX;
			public int MaxY;

			public Rect (OS.Rect rect)
			{
				MinX = rect.MinX;
				MinY = rect.MinY;
				MaxX = rect.MaxX;
				MaxY = rect.MaxY;
			}
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct Coord
		{
			public int X;
			public int Y;

			public Coord (OS.Coord coord)
			{
				X = coord.X;
				Y = coord.Y;
			}
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct Matrix
		{
			// Individual ints are easier to marshal than a 3x2 array and this
			// is how the PRMs explain it.
			public int a; // x scale factor, or cos(angle) to rotate
			public int b; // sin(angle) to rotate
			public int c; // -sin(angle) to rotate
			public int d; // y scale factor, or cos(angle) to rotate
			public int e; // x translation
			public int f; // y translation

			// Construct a native transformation matrix from a managed one
			public Matrix (OS.Matrix matrix)
			{
				a = matrix.Cell00;
				b = matrix.Cell01;
				c = matrix.Cell10;
				d = matrix.Cell11;
				e = matrix.Cell20;
				f = matrix.Cell21;
			}
		}
	}

	internal static partial class NativeMethods
	{
		// OS SWIs
		[DllImport("libriscosdotnet.so.1", EntryPoint="xos_plot")]
		internal static extern void OS_Plot (int command, int x, int y);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xoswordreadclock_utc")]
		internal static extern void OSWord_ReadClockUtc ([In,Out,MarshalAs(UnmanagedType.LPArray, SizeConst=5)]
								    byte [] utc);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xos_read_monotonic_time")]
		internal static extern void OS_ReadMonotonicTime (out uint time);

		// RISC OS 3.5+
		[DllImport("libriscosdotnet.so.1", EntryPoint="xosdynamicarea_create")]
		internal static extern IntPtr OS_DynamicAreaCreate (int number,
								    int size,
								    int baseAddress,
								    OS.DynamicAreaFlags flags,
								    int sizeLimit,
								    IntPtr handler,
								    IntPtr workspace,
								    string name,
								    out int number_out,
								    out IntPtr baseAddress_out,
								    out int sizeLimit_out);

		// RISC OS 3.5+
		[DllImport("libriscosdotnet.so.1", EntryPoint="xosdynamicarea_delete")]
		internal static extern IntPtr OS_DynamicAreaDelete (int number);

		// RISC OS 3.5+
		[DllImport("libriscosdotnet.so.1", EntryPoint="xosdynamicarea_read")]
		internal static extern IntPtr OS_DynamicAreaRead (int number,
								  out int size,
								  out IntPtr baseAddress,
								  out OS.DynamicAreaFlags flags,
								  out int sizeLimit,
								  out IntPtr handler,
								  out IntPtr workspace,
								  out IntPtr name);

		// RISC OS 3.5+
		[DllImport("libriscosdotnet.so.1", EntryPoint="xosdynamicarea_enumerate")]
		internal static extern IntPtr OS_DynamicAreaEnumerate (int context, out int context_out);

		// RISC OS 3.5+
		[DllImport("libriscosdotnet.so.1", EntryPoint="xosdynamicarea_renumber")]
		internal static extern IntPtr OS_DynamicAreaRenumber (int old_number, int new_number);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xos_change_dynamic_area")]
		internal static extern IntPtr OS_ChangeDynamicArea (int number,
								    int sizeIncrease,
								    out uint sizeIncrease_out);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosheap_initialise")]
		internal static extern IntPtr OS_HeapInitialise (IntPtr heap, int size);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosheap_describe")]
		internal static extern IntPtr OS_HeapDescribe (IntPtr heap,
							       out int largestBlockSize,
							       out int totalFree);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosheap_alloc")]
		internal static extern IntPtr OS_HeapAlloc (IntPtr heap,
							    int size,
							    out IntPtr block);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosheap_free")]
		internal static extern IntPtr OS_HeapFree (IntPtr heap, IntPtr block);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosheap_realloc")]
		internal static extern IntPtr OS_HeapRealloc (IntPtr heap,
							      IntPtr block,
							      int sizeIncrease,
							      out IntPtr block_out);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosheap_resize")]
		internal static extern IntPtr OS_HeapResize (IntPtr heap,
							     int sizeIncrease);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosheap_read_size")]
		internal static extern IntPtr OS_HeapReadSize (IntPtr heap,
							       IntPtr block,
							       out int size);

		// OS_Find
		[DllImport("libriscosdotnet.so.1", EntryPoint="xosfind_openinw")]
		internal static extern IntPtr OSFind_Openin (OSFind.Flags flags,
							     string filename,
							     string path,
							     out IntPtr handleOut);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosfind_openoutw")]
		internal static extern IntPtr OSFind_Openout (OSFind.Flags flags,
							      string filename,
							      string path,
							      out IntPtr handleOut);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosfind_openupw")]
		internal static extern IntPtr OSFind_Openup (OSFind.Flags flags,
							     string filename,
							     string path,
							     out IntPtr handleOut);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosfind_closew")]
		internal static extern IntPtr OSFind_Close (IntPtr fileHandle);

		// OS_Args
		[DllImport("libriscosdotnet.so.1", EntryPoint="xosargs_read_ptrw")]
		internal static extern IntPtr OSArgs_ReadFilePtr (IntPtr fileHandle,
								  out int filePtrOut);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosargs_set_ptrw")]
		internal static extern IntPtr OSArgs_SetFilePtr (IntPtr fileHandle,
								 int filePtr);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosargs_read_extw")]
		internal static extern IntPtr OSArgs_ReadFileExt (IntPtr fileHandle,
								  out int fileExtOut);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosargs_set_extw")]
		internal static extern IntPtr OSArgs_SetFileExt (IntPtr fileHandle,
								 int fileExt);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosargs_read_allocationw")]
		internal static extern IntPtr OSArgs_ReadFileAllocation (IntPtr fileHandle,
									 out int fileAllocationOut);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosargs_read_eof_statusw")]
		internal static extern IntPtr OSArgs_ReadFileEOF (IntPtr fileHandle,
								  out int EOF);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosargs_set_allocationw")]
		internal static extern IntPtr OSArgs_SetFileAllocation (IntPtr fileHandle,
									int fileAllocation,
									out int fileAllocationOut);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosargs_read_pathw")]
		internal static extern IntPtr OSArgs_ReadFilename (IntPtr fileHandle,
								   System.Text.StringBuilder buffer,
								   int bufferSize,
								   out int spareOut);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosargs_read_infow")]
		internal static extern IntPtr OSArgs_ReadInfo (IntPtr fileHandle,
							       out int streamStatus,
							       out int infoWord);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosargs_ensurew")]
		internal static extern IntPtr OSArgs_EnsureData (IntPtr fileHandle);

		// OS_GBPB
		[DllImport("libriscosdotnet.so.1", EntryPoint="xosgbpb_write_atw")]
		internal static extern IntPtr OSGBPB_WriteAt (IntPtr fileHandle,
							      IntPtr data,
							      int size,
							      int filePtr,
							      out int unwritten);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosgbpb_writew")]
		internal static extern IntPtr OSGBPB_Write (IntPtr fileHandle,
							    IntPtr data,
							    int size,
							    out int unwritten);


		[DllImport("libriscosdotnet.so.1", EntryPoint="xosgbpb_read_atw")]
		internal static extern IntPtr OSGBPB_ReadAt (IntPtr fileHandle,
							     IntPtr data,
							     int size,
							     int filePtr,
							     out int unread);

		[DllImport("libriscosdotnet.so.1", EntryPoint="xosgbpb_readw")]
		internal static extern IntPtr OSGBPB_Read (IntPtr fileHandle,
							    IntPtr data,
							    int size,
							    out int unread);

		[DllImport("libriscosdotnet.so.1", EntryPoint="rdn_OS_BPut")]
		internal static extern IntPtr OS_BPut (int value, IntPtr fileHandle);

		[DllImport("libriscosdotnet.so.1", EntryPoint="rdn_OS_BGet")]
		internal static extern IntPtr OS_BGet (IntPtr fileHandle,
						       out int value,
						       out bool carry_flag);
	}
}

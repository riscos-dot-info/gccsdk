--- pango/opentype/hb-open-type-private.hh.orig	2009-11-26 00:44:17.000000000 +0000
+++ pango/opentype/hb-open-type-private.hh	2009-12-27 11:47:48.000000000 +0000
@@ -356,7 +356,7 @@
       return SANITIZE_SELF (); \
     } \
     private: unsigned char v[BYTES]; \
-  }; \
+  } PACKED; \
   ASSERT_SIZE (NAME, BYTES)
 
 #define DEFINE_INT_TYPE1(NAME, TYPE, BIG_ENDIAN, BYTES) \
@@ -370,7 +370,7 @@
       return SANITIZE_SELF (); \
     } \
     private: unsigned char v[BYTES]; \
-  }; \
+  } PACKED; \
   ASSERT_SIZE (NAME, BYTES)
 #define DEFINE_INT_TYPE0(NAME, type, b)	DEFINE_INT_TYPE1 (NAME, type##_t, hb_be_##type, b)
 #define DEFINE_INT_TYPE(NAME, u, w)	DEFINE_INT_TYPE0 (NAME, u##int##w, (w / 8))
@@ -402,7 +402,7 @@
      */
     return SANITIZE_SELF () && (((uint32_t) *this) & 0x80808080) == 0;
   }
-};
+} PACKED;
 ASSERT_SIZE (Tag, 4);
 #define _NULL_TAG_INIT  {' ', ' ', ' ', ' '}
 DEFINE_NULL_DATA (Tag, 4, _NULL_TAG_INIT);
@@ -430,7 +430,7 @@
       Sum += *Table++;
     return Sum;
   }
-};
+} PACKED;
 ASSERT_SIZE (CheckSum, 4);
 
 
@@ -449,7 +449,7 @@
 
   USHORT major;
   USHORT minor;
-};
+} PACKED;
 ASSERT_SIZE (FixedVersion, 4);
 
 
@@ -490,15 +490,15 @@
     if (HB_UNLIKELY (!offset)) return true;
     return SANITIZE_BASE (CAST(Type, *DECONST_CHARP(base), offset), user_data) || NEUTER (DECONST_CAST(OffsetType,*this,0), 0);
   }
-};
+} PACKED;
 template <typename Base, typename OffsetType, typename Type>
 inline const Type& operator + (const Base &base, GenericOffsetTo<OffsetType, Type> offset) { return offset (base); }
 
 template <typename Type>
-struct OffsetTo : GenericOffsetTo<Offset, Type> {};
+struct OffsetTo : GenericOffsetTo<Offset, Type> {} PACKED;
 
 template <typename Type>
-struct LongOffsetTo : GenericOffsetTo<LongOffset, Type> {};
+struct LongOffsetTo : GenericOffsetTo<LongOffset, Type> {} PACKED;
 
 
 /*
@@ -563,27 +563,27 @@
 
   LenType len;
 /*Type array[VAR];*/
-};
+} PACKED;
 
 /* An array with a USHORT number of elements. */
 template <typename Type>
-struct ArrayOf : GenericArrayOf<USHORT, Type> {};
+struct ArrayOf : GenericArrayOf<USHORT, Type> {} PACKED;
 
 /* An array with a ULONG number of elements. */
 template <typename Type>
-struct LongArrayOf : GenericArrayOf<ULONG, Type> {};
+struct LongArrayOf : GenericArrayOf<ULONG, Type> {} PACKED;
 
 /* Array of Offset's */
 template <typename Type>
-struct OffsetArrayOf : ArrayOf<OffsetTo<Type> > {};
+struct OffsetArrayOf : ArrayOf<OffsetTo<Type> > {} PACKED;
 
 /* Array of LongOffset's */
 template <typename Type>
-struct LongOffsetArrayOf : ArrayOf<LongOffsetTo<Type> > {};
+struct LongOffsetArrayOf : ArrayOf<LongOffsetTo<Type> > {} PACKED;
 
 /* LongArray of LongOffset's */
 template <typename Type>
-struct LongOffsetLongArrayOf : LongArrayOf<LongOffsetTo<Type> > {};
+struct LongOffsetLongArrayOf : LongArrayOf<LongOffsetTo<Type> > {} PACKED;
 
 /* Array of offsets relative to the beginning of the array itself. */
 template <typename Type>
@@ -603,7 +603,7 @@
     TRACE_SANITIZE ();
     return OffsetArrayOf<Type>::sanitize (SANITIZE_ARG, CONST_CHARP(this), user_data);
   }
-};
+} PACKED;
 
 
 /* An array with a USHORT number of elements,
@@ -640,7 +640,7 @@
 
   USHORT len;
 /*Type array[VAR];*/
-};
+} PACKED;
 
 
 #endif /* HB_OPEN_TYPES_PRIVATE_HH */
#ifndef CPARAMUTILS_H
#define CPARAMUTILS_H

typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef short wchar_t;
typedef uint64_t size_t;
typedef int BOOL;

// Not really required for interfacing with CParamUtils, but nice to have in general
extern void* malloc(size_t size);
extern void* calloc(size_t num, size_t size);
extern void* realloc(void* ptr, size_t new_size);
extern void free(void* ptr);
extern void* memcpy(void* destination, const void* source, size_t num);
extern void memset(void* dest, uint8_t val, size_t size);
extern char* strdup(const char* str1);
extern size_t strlen(const char* str);
extern int strcmp(const char* s1, const char* s2);
extern int wcscmp(const wchar_t* s1, const wchar_t* s2);

typedef struct _ParamRowInfo
{
	uint64_t row_id; // ID of param row
	uint64_t param_offset; // Offset of pointer to param data relative to parent table
	uint64_t param_end_offset; // Seems to point to end of ParamTable struct
} ParamRowInfo;

typedef struct _ParamTable
{
	uint8_t pad00[0x00A];
	uint16_t num_rows; // Number of rows in param table

	uint8_t pad01[0x004];
	uint64_t param_type_offset; // Offset of param type string from the beginning of this struct

	uint8_t pad02[0x028];
	ParamRowInfo rows[0]; // Array of row information structs
} ParamTable;

// struct holding processed game param information
typedef struct _param_info
{
	wchar_t* name;
	size_t index;
	char* type;
	size_t row_size;
	ParamTable* table;
	void* _reserved;
} param_info;

// Callback receiving the param name and table, respectively.
// Returning TRUE (1) will terminate the iteration.
typedef BOOL (*param_iter_func)(wchar_t*, ParamTable*);

// Callback receiving the row ID and address, respectively.
// Returning TRUE (1) will terminate the iteration.
typedef BOOL (*row_iter_func)(uint64_t, void*);

// Iterate over game params.
extern void CParamUtils_ParamIterator(param_iter_func cb);

// Iterate over the rows of a param. Returns FALSE (0) if param doesn't exist.
extern BOOL CParamUtils_RowIterator(wchar_t* param_name, row_iter_func cb);

// Get a pointer to processed param info given a game param. NULL if param doesn't exist.
extern param_info* CParamUtils_GetParamInfo(wchar_t* param_name);

// Like CParamUtils_GetParamInfo, but prints out an error message in the CE console on failure. 
extern param_info* CParamUtils_GetParamInfoVerbose(wchar_t* param_name);

// Return the index of a param row given it's row ID (-1 if not found).
extern int32_t CParamUtils_GetRowIndex(wchar_t* param_name, uint64_t row_id);

// Like CParamUtils_GetRowIndex, but prints out an error message in the CE console on failure. 
extern int32_t CParamUtils_GetRowIndexVerbose(wchar_t* param_name, uint64_t row_id);

// Get a pointer to the row data for a given param, by row ID. NULL if ID/param doesn't exist.
extern void* CParamUtils_GetRowData(wchar_t* param_name, uint64_t row_id);

// Like CParamUtils_GetRowData, but prints out an error message in the CE console on failure. 
extern void* CParamUtils_GetRowDataVerbose(wchar_t* param_name, uint64_t row_id);

/* Param Patcher internal calls, should never be used directly. Use the macros instead. */

// If a previous call to BeginNamedPatch specified a valid name, returns the current patch object
// instance. Otherwise, returns a null pointer.
extern void* CParamUtils_Internal_GetPatchIns(BOOL debug);

// Begin a memory patch, and return a pointer to the given param row's data.
extern void* CParamUtils_Internal_BeginRowPatch(int32_t param_index, int32_t row_index);

// Call immediately after having called BeginPatch and having modified the returned param row. 
extern void CParamUtils_Internal_FinalizeRowPatch(void* h_patch, int32_t param_index, int32_t row_index);

// Attempts to restore a named param patch. Returns FALSE if the patch was not found.
extern BOOL CParamUtils_Internal_RestorePatch(const char* name, BOOL debug);

// Declare a new named param patch. This acquires a critical section, so CParamUtils_Internal_FinalizeNamedPatch 
// MUST be called after all desired patches have been applied to release it.
extern void CParamUtils_Internal_BeginNamedPatch(const char* name, BOOL debug);

// Signify the current named patch is complete. Release the internal param patcher lock, and
// prevents any future patches from being made under this name until the script is disabled. 
extern void CParamUtils_Internal_FinalizeNamedPatch();

/* Param Patcher macros */

#define ParamPatch(param_name, row_id, body) { \
	param_info* __p_info = CParamUtils_GetParamInfoVerbose(L ## #param_name); \
	int32_t __row_index = CParamUtils_GetRowIndexVerbose(L ## #param_name, row_id); \
	void* __patch; \
	if (__p_info && __row_index != -1 && (__patch = CParamUtils_Internal_GetPatchIns(1))) { \
		param_name* param = CParamUtils_Internal_BeginRowPatch(__p_info->index, __row_index); \
		if (param) body; \
		CParamUtils_Internal_FinalizeRowPatch(__patch, __p_info->index, __row_index); \
	} \
}

#define ParamPatchAll(param_name, body) { \
	param_info* __p_info = CParamUtils_GetParamInfoVerbose(L ## #param_name); \
	uint16_t __num_rows; \
	void* __patch; \
	if (__p_info && (__num_rows =__p_info->table->num_rows) && (__patch = CParamUtils_Internal_GetPatchIns(1))) { \
		for (uint16_t __row_index = 0; __row_index < __num_rows; __row_index++) { \
			param_name* param = CParamUtils_Internal_BeginRowPatch(__p_info->index, __row_index); \
			if (param) body; \
			CParamUtils_Internal_FinalizeRowPatch(__patch, __p_info->index, __row_index); \
		} \
	} \
}

#define ParamPatchBegin(name) CParamUtils_Internal_BeginNamedPatch(name, 1)
#define ParamPatchEnd() CParamUtils_Internal_FinalizeNamedPatch()
#define ParamRestore(name) CParamUtils_Internal_RestorePatch(name, 1)

#endif
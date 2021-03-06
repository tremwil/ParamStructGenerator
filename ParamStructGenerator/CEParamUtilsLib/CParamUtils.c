// comment out for memory allocation debugging
//#define ALLOC_DEBUG

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
typedef uint64_t uintptr_t;
typedef int BOOL;

#define NULL ((void*)0)
#define TRUE ((BOOL)1)
#define FALSE ((BOOL)0)

extern void* malloc(size_t size);
extern void* calloc(size_t num, size_t size);
extern void* realloc(void* ptr, size_t new_size);
extern char* strdup(const char* str);
extern void free(void* ptr);

extern void* aligned_malloc(size_t size, size_t alignment);
extern void* aligned_realloc(void* ptr, size_t new_size, size_t alignment);
extern void aligned_free(void* ptr);

extern void* memcpy(void* destination, const void* source, size_t num);
extern void memset(void* dest, uint8_t val, size_t size);
extern size_t strlen(const char* str);
extern int strcmp(const char* s1, const char* s2);
extern int wcscmp(const wchar_t* s1, const wchar_t* s2);

extern void* CELUA_ExecuteFunction(char* luacode, void* param);
extern void CELUA_ExecuteFunctionAsync(char* luacode, void* param);

typedef struct _CRITICAL_SECTION
{
	uint8_t reserved[40];
} CRITICAL_SECTION, * LPCRITICAL_SECTION;

extern void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
extern void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
extern BOOL TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
extern void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

/* (de)allocator macros for debugging */

static BOOL MEM_LOG_ENABLED = FALSE;

#define MALLOC malloc
#define CALLOC calloc
#define REALLOC realloc
#define STRDUP strdup
#define FREE free

#define ALIGNED_MALLOC aligned_malloc
#define ALIGNED_REALLOC aligned_realloc
#define ALIGNED_FREE aligned_free

#ifdef ALLOC_DEBUG

#define FUN_LOG1(fun, ret_type, arg_fmt, ret_fmt, arg1) \
    ret_type __ret = fun(arg1); \
	if (MEM_LOG_ENABLED) { \
		char __buff[1024]; \
		snprintf(__buff, sizeof(__buff), "print(\"" #fun "(" arg_fmt ") -> " ret_fmt "\")", arg1, __ret); \
		CELUA_ExecuteFunctionAsync(__buff, 0); \
	} \
    return __ret; \


#define FUN_LOG2(fun, ret_type, arg_fmt, ret_fmt, arg1, arg2) \
    ret_type __ret = fun(arg1, arg2); \
	if (MEM_LOG_ENABLED) { \
		char __buff[1024]; \
		snprintf(__buff, sizeof(__buff), "print(\"" #fun "(" arg_fmt ") -> " ret_fmt "\")", arg1, arg2, __ret); \
		CELUA_ExecuteFunctionAsync(__buff, 0); \
	} \
    return __ret; \


#define FUN_LOG3(fun, ret_type, arg_fmt, ret_fmt, arg1, arg2, arg3) \
    ret_type __ret = fun(arg1, arg2, arg3); \
	if (MEM_LOG_ENABLED) { \
		char __buff[1024]; \
		snprintf(__buff, sizeof(__buff), "print(\"" #fun "(" arg_fmt ") -> " ret_fmt "\")", arg1, arg2, arg3, __ret); \
		CELUA_ExecuteFunctionAsync(__buff, 0); \
	} \
    return __ret; \

#define FUN_LOG_VOID1(fun, arg_fmt, arg1) \
    fun(arg1); \
	if (MEM_LOG_ENABLED) { \
		char __buff[1024]; \
		snprintf(__buff, sizeof(__buff), "print(\"" #fun "(" arg_fmt ") -> void\")", arg1); \
		CELUA_ExecuteFunctionAsync(__buff, 0); \
	} \

void* dbg_malloc(size_t size) { FUN_LOG1(malloc, void*, "%lld", "%p", size); }
void* dbg_calloc(size_t num, size_t size) { FUN_LOG2(calloc, void*, "%lld, %lld", "%p", num, size); }
void* dbg_realloc(void* ptr, size_t new_size) { FUN_LOG2(realloc, void*, "%p, %lld", "%p", ptr, new_size); }
char* dbg_strdup(const char* str) { FUN_LOG1(strdup, char*, "%p", "%p", str); }
void dbg_free(void* ptr) { FUN_LOG_VOID1(free, "%p", ptr); }

void* dbg_aligned_malloc(size_t size, size_t alignment) { FUN_LOG2(aligned_malloc, void*, "%lld, %lld", "%p", size, alignment); }
void* dbg_aligned_realloc(void* ptr, size_t new_size, size_t alignment) { FUN_LOG3(aligned_realloc, void*, "%p, %lld, %lld", "%p", ptr, new_size, alignment); };
void dbg_aligned_free(void* ptr) { FUN_LOG_VOID1(aligned_free, "%p", ptr); }

#define MALLOC dbg_malloc
#define CALLOC dbg_calloc
#define REALLOC dbg_realloc
#define STRDUP dbg_strdup
#define FREE dbg_free

#define ALIGNED_MALLOC dbg_aligned_malloc
#define ALIGNED_REALLOC dbg_aligned_realloc
#define ALIGNED_FREE dbg_aligned_free

#endif

/* Game structs/classes */

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

// Get the param type string for this param table.
inline char* get_param_type(ParamTable* tbl)
{
	return (char*)tbl + tbl->param_type_offset;
}
// Get a pointer to the data of a given row of this param.
inline uint8_t* get_row_data(ParamTable* tbl, int index)
{
	return (char*)tbl + tbl->rows[index].param_offset;
}

// Compute the size of this param using the difference between the param type offset and last row offset.
// 0 if the param has no rows.
inline uint64_t get_param_size(ParamTable* tbl)
{
	return (tbl->num_rows == 0) ? 0 : tbl->param_type_offset - tbl->rows[tbl->num_rows - 1].param_offset;
}

typedef struct _ParamHeader
{
	uint8_t pad00[0x80];
	ParamTable* param_table;
} ParamHeader;

typedef struct _DLWString
{
	union
	{
		wchar_t in_place[8];
		wchar_t* ptr;
	} str;
	uint64_t length;
	uint64_t capacity;
} DLWString;

inline wchar_t* dlw_c_str(DLWString* s)
{
	return (s->capacity > 7) ? s->str.ptr : s->str.in_place;
}

typedef struct _ParamResCap
{
	void** vftable_ptr;

	uint8_t pad00[0x10];
	DLWString param_name;

	uint8_t pad01[0x48];
	ParamHeader* param_header;
} ParamResCap;

typedef struct _CSRegulationManagerImp
{
	void** vftable_ptr;

	uint8_t pad00[0x10];
	ParamResCap** param_list_begin;
	ParamResCap** param_list_end;
} CSRegulationManagerImp;

/* Simple fixed bucket size hashmap implementation to speed up param and row lookups */

typedef uint64_t(*hash_fun)(void*);
typedef BOOL(*eq_fun)(void*, void*);

typedef struct _hm_node
{
	struct _hm_node* next;

	void* key;
	void* value;
} hm_node;

typedef struct _hashmap
{
	hm_node** buckets;
	size_t num_buckets;

	hash_fun hash;
	eq_fun eq;

	BOOL free_keys; // if true, will free keys on delete
	BOOL free_values; // if true, will free values on delete
} hashmap;

uint64_t u64_hash(uint64_t x)
{
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
	x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
	x = x ^ (x >> 31);
	return x;
}
BOOL u64_eq(uint64_t a, uint64_t b) { return a == b; }

uint32_t str_hash(const char* str)
{
	uint32_t hash = 5381, c;
	uint8_t* ustr = (uint8_t*)str;

	while (c = *ustr++) hash = ((hash << 5) + hash) + c;
	return hash;
}
BOOL str_eq(const char* a, const char* b) { return !strcmp(a, b); }

uint32_t wstr_hash(const wchar_t* str)
{
	uint32_t hash = 5381, c;
	uint8_t* ustr = (uint8_t*)str;

	while ((c = *ustr++) || *ustr) hash = ((hash << 5) + hash) + c;
	return hash;
}
BOOL wstr_eq(const wchar_t* a, const wchar_t* b) { return !wcscmp(a, b); }

hashmap* hm_create(size_t num_buckets, hash_fun hash, eq_fun eq, BOOL free_keys, BOOL free_values)
{
	hashmap* hm = malloc(sizeof(hashmap));
	hm->buckets = calloc(num_buckets, sizeof(hm_node*));
	hm->num_buckets = num_buckets;
	hm->hash = hash;
	hm->eq = eq;
	hm->free_keys = free_keys;
	hm->free_values = free_values;
	return hm;
}

hm_node* hm_set(hashmap* hm, void* key, void* value)
{
	uint64_t i = hm->hash(key) % hm->num_buckets;
	hm_node* node = hm->buckets[i];
	for (; node != NULL && !hm->eq(node->key, key); node = node->next);

	if (node == NULL)
	{
		node = malloc(sizeof(hm_node));
		node->next = hm->buckets[i];
		hm->buckets[i] = node;
	}
	else
	{
		if (hm->free_keys) free(node->key);
		if (hm->free_values) free(node->value);
	}

	node->key = key;
	node->value = value;
	return node;
}

hm_node* hm_get_node(hashmap* hm, void* key)
{
	uint64_t i = hm->hash(key) % hm->num_buckets;
	hm_node* node = hm->buckets[i];
	for (; node != NULL && !hm->eq(node->key, key); node = node->next);
	return node;
}

inline void* hm_get_val(hashmap* hm, void* key)
{
	hm_node* node = hm_get_node(hm, key);
	return (node == NULL) ? NULL : node->value;
}

typedef BOOL(*hm_iterate_fun)(void*, void*);
void hm_iterate(hashmap* hm, hm_iterate_fun f)
{
	for (size_t i = 0; i < hm->num_buckets; i++)
	{
		if (hm->buckets[i] != NULL)
		{
			hm_node* node = hm->buckets[i];
			for (; node != NULL && !f(node->key, node->value); node = node->next);
			if (node != NULL) return;
		}
	}
}

inline BOOL hm_exists(hashmap* hm, void* key)
{
	return hm_get_node(hm, key) != NULL;
}

BOOL hm_remove(hashmap* hm, void* key)
{
	uint64_t i = hm->hash(key) % hm->num_buckets;
	hm_node* node = hm->buckets[i], * prev = NULL;
	for (; node != NULL && !hm->eq(node->key, key); node = (prev = node)->next);

	if (node != NULL)
	{
		if (hm->free_keys) free(node->key);
		if (hm->free_values) free(node->value);

		if (prev != NULL) prev->next = node->next;
		else hm->buckets[i] = node->next;

		free(node);
		return TRUE;
	}
	else return FALSE;
}

void hm_free(hashmap* hm)
{
	for (size_t i = 0; i < hm->num_buckets; i++)
	{
		hm_node* node = hm->buckets[i];
		if (node != NULL)
		{
			if (hm->free_keys) free(node->key);
			if (hm->free_values) free(node->value);
			FREE(node);
		}
	}
	free(hm->buckets);
	free(hm);
}

/* ParamUtils API */

// struct holding processed game param information
typedef struct _param_info
{
	wchar_t* name;
	size_t index;
	char* type;
	size_t row_size;
	ParamTable* table;
	hashmap* row_id_map;
} param_info;

extern CSRegulationManagerImp* CSRegulationManager;
hashmap* PARAM_MAP = 0;

// Callback receiving the param name and table, respectively.
// Returning TRUE (1) will terminate the iteration.
typedef BOOL(*param_iter_func)(wchar_t*, ParamTable*);

// Callback receiving the row ID and address, respectively.
// Returning TRUE (1) will terminate the iteration.
typedef BOOL(*row_iter_func)(uint64_t, void*);

// Iterate over game params.
void CParamUtils_ParamIterator(param_iter_func cb)
{
	uint64_t num_params = CSRegulationManager->param_list_end - CSRegulationManager->param_list_begin;
	for (size_t i = 0; i < num_params; i++)
	{
		ParamResCap* res_cap = CSRegulationManager->param_list_begin[i];

		if (cb(dlw_c_str(&res_cap->param_name), res_cap->param_header->param_table))
			break;
	}
}

// Iterate over the rows of a param. Returns FALSE (0) if param doesn't exist.
BOOL CParamUtils_RowIterator(wchar_t* param_name, row_iter_func cb)
{
	hm_node* node = hm_get_node(PARAM_MAP, param_name);
	if (node == 0) return FALSE;

	ParamTable* tbl = ((param_info*)node->value)->table;
	for (int j = 0; j < tbl->num_rows; j++)
	{
		if (cb(tbl->rows[j].row_id, get_row_data(tbl, j)))
			break;
	}
	return TRUE;
}

// Get a pointer to processed param info given a game param. NULL if param doesn't exist.
param_info* CParamUtils_GetParamInfo(wchar_t* param_name)
{
	hm_node* node = hm_get_node(PARAM_MAP, param_name);
	return (node == NULL) ? NULL : (param_info*)node->value;
}

// Convert an UTF-16 string in the ASCII range to a single byte string.
size_t wchar_to_char(char* dest_buff, size_t cb, const wchar_t* wide_str)
{
	char* start = dest_buff;
	for (char* end = start + cb - 1; dest_buff < end && (*wide_str & 0xFF) != 0; dest_buff++, wide_str++)
		*dest_buff = *wide_str;

	*dest_buff = 0;
	return dest_buff - start;
}

// Like CParamUtils_GetParamInfo, but prints out an error message in the CE console on failure.
param_info* CParamUtils_GetParamInfoVerbose(wchar_t* param_name)
{
	param_info* res = CParamUtils_GetParamInfo(param_name);
	if (res == NULL)
	{
		char cstr[512];
		wchar_to_char(cstr, sizeof(cstr), param_name);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "print(\"CParamUtils Error: Param \\\"%s\\\" not found\")", cstr);
		CELUA_ExecuteFunctionAsync(buffer, 0);
	}
	return res;
}

// Return the index of a param row given it's row ID (-1 if not found).
int32_t CParamUtils_GetRowIndex(wchar_t* param_name, uint64_t row_id)
{
	param_info* pinfo = (param_info*)hm_get_val(PARAM_MAP, param_name);
	if (pinfo == NULL) return NULL;

	hm_node* row_node = hm_get_node(pinfo->row_id_map, (void*)row_id);
	return (row_node == NULL) ? -1 : (int32_t)row_node->value;
}

// Like CParamUtils_GetRowIndex, but prints out an error message in the CE console on failure.
extern int32_t CParamUtils_GetRowIndexVerbose(wchar_t* param_name, uint64_t row_id)
{
	int32_t res = CParamUtils_GetRowIndex(param_name, row_id);
	if (res == -1)
	{
		char cstr[512];
		wchar_to_char(cstr, sizeof(cstr), param_name);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "print(\"CParamUtils Error: Row ID %I64d of param \\\"%s\\\" not found\")", row_id, cstr);
		CELUA_ExecuteFunctionAsync(buffer, 0);
	}
	return res;
}

// Get a pointer to the row data for a given param, by row ID. NULL if ID/param doesn't exist.
void* CParamUtils_GetRowData(wchar_t* param_name, uint64_t row_id)
{
	param_info* pinfo = (param_info*)hm_get_val(PARAM_MAP, param_name);
	if (pinfo == NULL) return NULL;

	hm_node* row_node = hm_get_node(pinfo->row_id_map, (void*)row_id);
	return (row_node == NULL) ? NULL : get_row_data(pinfo->table, (int32_t)row_node->value);
}

// Like CParamUtils_GetRowData, but prints out an error message in the CE console on failure.
extern void* CParamUtils_GetRowDataVerbose(wchar_t* param_name, uint64_t row_id)
{
	void* res = CParamUtils_GetRowData(param_name, row_id);
	if (res == NULL)
	{
		char cstr[512];
		wchar_to_char(cstr, sizeof(cstr), param_name);

		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "print(\"CParamUtils Error: Row ID %I64d of param \\\"%s\\\" not found\")", row_id, cstr);
		CELUA_ExecuteFunctionAsync(buffer, 0);
	}
	return res;
}

/* Memory patching system */

// Optimized subroutines for bitwise operations on aligned memory.
// TCC has no optimization, so we have to use simd manually
void asm_defs()
{
	__asm__ __volatile__("

		memxor_simd:
	xorq % rax, % rax
		loop_xor_simd :
	movaps(% rcx, % rax), % xmm0
		pxor(% rdx, % rax), % xmm0
		movaps % xmm0, (% rcx, % rax)
		addq $0x10, % rax
		cmpq % rax, % r8
		ja loop_xor_simd
		ret

		memor_simd :
	xorq % rax, % rax
		loop_or_simd :
	movaps(% rcx, % rax), % xmm0
		por(% rdx, % rax), % xmm0
		movaps % xmm0, (% rcx, % rax)
		addq $0x10, % rax
		cmpq % rax, % r8
		ja loop_or_simd
		ret

		memandn_simd :
	xorq % rax, % rax
		loop_andn_simd :
	movaps(% rcx, % rax), % xmm0
		pandn(% rdx, % rax), % xmm0
		movaps % xmm0, (% rcx, % rax)
		addq $0x10, % rax
		cmpq % rax, % r8
		ja loop_andn_simd
		ret

		memand_simd :
	xorq % rax, % rax
		loop_and_simd :
	movaps(% rcx, % rax), % xmm0
		pand(% rdx, % rax), % xmm0
		movaps % xmm0, (% rcx, % rax)
		addq $0x10, % rax
		cmpq % rax, % r8
		ja loop_and_simd
		ret"
		);
}
extern void memxor_simd(void* dest, void* src, size_t len);
extern void memor_simd(void* dest, void* src, size_t len);
extern void memandn_simd(void* dest, void* src, size_t len);
extern void memand_simd(void* dest, void* src, size_t len);

#define align16(n) (((n) + 0xF) & ~(uint64_t)0xF)

typedef struct _field_bitmask
{
	uint32_t offset;
	uint32_t uid; // to handle fields wider than 8 bytes (i.e. arrays). uid 0 indicates the terminator field
	uint64_t bitmask;
} field_bitmask;

typedef struct _mem_diff
{
	struct _mem_diff* next;
	void* diff_mask; // 16-byte aligned mask of bitwise diff (xor) between old/new mem
	void* field_mask; // 16-byte aligned mask of affected field bits
	uint32_t patch_uid; // unique id of related named patch, used to detect if merging is possible
} mem_diff;

// Stack of changes to a given memory block (in this case, param rows)
typedef struct _mem_diff_stack
{
	size_t region_size; // Region size, including extra from alignment requirements
	field_bitmask* field_defs;
	mem_diff* diff_stack_top;

	void* live_mem; // Target memory this diff will affect
	void* aligned_live_mem; // live_mem, aligned to 16 bytes
	void* temp_buffer; // 16-byte aligned
} mem_diff_stack;

// Create a mem diff stack tracking changes to a given block.
mem_diff_stack* mds_init(mem_diff_stack* mds, void* live_mem, size_t region_size, field_bitmask* field_defs)
{
	mds->region_size = align16(region_size + ((uintptr_t)live_mem & 0xF));
	mds->field_defs = field_defs;
	mds->diff_stack_top = NULL;
	mds->live_mem = live_mem;
	mds->aligned_live_mem = (uintptr_t)live_mem & ~(uintptr_t)0xF;
	mds->temp_buffer = ALIGNED_MALLOC(mds->region_size, 16);
	return mds;
}

// Register a new patch to the memory region. Will return a pointer to the live memory for modifications.
// Make sure to call mds_push_end immediately after modifications to save the diff.
void* mds_push_begin(mem_diff_stack* mds)
{
	memcpy(mds->temp_buffer, mds->aligned_live_mem, mds->region_size);
	return mds->live_mem;
}

// Finalize pushing a new memory change to the stack. If a new diff needs to be created,
// will return a pointer to it. Otherwise, returns NULL.
mem_diff* mds_push_end(mem_diff_stack* mds, uint32_t patch_uid)
{
	memxor_simd(mds->temp_buffer, mds->aligned_live_mem, mds->region_size);
	mem_diff* new_diff = NULL;

	// If the top block has the same patch_uid, merge them instead of creating a new block
	if (mds->diff_stack_top != NULL && mds->diff_stack_top->patch_uid == patch_uid)
		memxor_simd(mds->diff_stack_top, mds->temp_buffer, mds->region_size);
	// Otherwise, allocate a new block
	else
	{
		new_diff = MALLOC(sizeof(mem_diff));
		new_diff->next = mds->diff_stack_top;
		new_diff->diff_mask = ALIGNED_MALLOC(mds->region_size, 16);
		new_diff->field_mask = ALIGNED_MALLOC(mds->region_size, 16);
		new_diff->patch_uid = patch_uid;
		memcpy(new_diff->diff_mask, mds->temp_buffer, mds->region_size);
		memcpy(new_diff->field_mask, mds->temp_buffer, mds->region_size);
		mds->diff_stack_top = new_diff;
	}
	// Update the field mask if present
	if (mds->field_defs != NULL)
	{
		field_bitmask* fields = mds->field_defs;
		uintptr_t align_offset = (uintptr_t)mds->live_mem - (uintptr_t)mds->aligned_live_mem;
		uint8_t* diff_mask = (uint8_t*)mds->diff_stack_top->diff_mask + align_offset;
		uint8_t* field_mask = (uint8_t*)mds->diff_stack_top->field_mask + align_offset;

		for (int i = 0; fields[i].uid != 0;)
		{
			if (*(uint64_t*)(diff_mask + fields[i].offset) & fields[i].bitmask)
			{
				*(uint64_t*)(field_mask + fields[i].offset) |= fields[i].bitmask;

				for (int j = i - 1; j >= 0 && fields[j].uid == fields[i].uid; j--)
					*(uint64_t*)(field_mask + fields[j].offset) |= fields[j].bitmask;

				while (fields[i].uid == fields[++i].uid)
					*(uint64_t*)(field_mask + fields[i].offset) |= fields[i].bitmask;
			}
			else i++;
		}
	}
	return new_diff;
}

// Undo all changes in a given diff which are visible to the live object.
// "visible" regions are those not modified by a later memory edit.
// If the diff is on the top of the stack, O(region_size). Otherwise,
// O(depth * region_size). Returns TRUE if diff was found, FALSE otherwise.
BOOL mds_restore(mem_diff_stack* mds, mem_diff* diff)
{
	if (mds->diff_stack_top == NULL) return FALSE;

	// If on top of stack, immediately apply changes to live memory and FREE the block
	if (mds->diff_stack_top == diff)
	{
		memxor_simd(mds->aligned_live_mem, diff->diff_mask, mds->region_size);
		ALIGNED_FREE(diff->diff_mask);
		ALIGNED_FREE(diff->field_mask);
		mds->diff_stack_top = diff->next;
		FREE(diff);
		return TRUE;
	}

	// Otherwise, compute "hidden" changes mask by ORing field masks above diff
	memcpy(mds->temp_buffer, mds->diff_stack_top->field_mask, mds->region_size);
	mem_diff* curr = mds->diff_stack_top->next, * prev = mds->diff_stack_top;
	for (; curr != NULL && curr != diff; curr = (prev = curr)->next)
		memor_simd(mds->temp_buffer, curr->field_mask, mds->region_size);

	if (curr == NULL) return FALSE;

	// Merge changes into the previous block
	memxor_simd(prev->diff_mask, curr->diff_mask, mds->region_size);
	memor_simd(prev->field_mask, curr->field_mask, mds->region_size);

	// Remove parts of merged changes that will be reverted now, i.e. only keep merged "hidden" changes
	memand_simd(prev->diff_mask, mds->temp_buffer, mds->region_size);
	memand_simd(prev->field_mask, mds->temp_buffer, mds->region_size);

	// ANDN "hidden" changes mask with diff mask and apply to live mem
	memandn_simd(mds->temp_buffer, diff->diff_mask, mds->region_size);
	memxor_simd(mds->aligned_live_mem, mds->temp_buffer, mds->region_size);

	// Free the unused block
	prev->next = curr->next;
	ALIGNED_FREE(curr->diff_mask);
	ALIGNED_FREE(curr->field_mask);
	FREE(curr);

	return TRUE;
}

/* Param Patcher API */

// Array of mem diffs, indexed by param index, row index
mem_diff_stack** MEM_DIFFS = NULL;

// Represents a collection of param patches under a given name
typedef struct _named_patch
{
	struct _named_patch* next;
	char* name;
	uint32_t uid; // Unique identifier for this named patch
	int32_t diffs_num;
	int32_t diffs_cap;

	mem_diff_stack** diff_stacks;
	mem_diff** diffs;
} named_patch;

#define NAMED_PATCH_INIT_DIFF_CAP 0x100

// Stack of named param patches defined by the user.
uint32_t NAMED_PATCH_UID_CTR = 0;
named_patch* NAMED_PATCH_LL_HEAD = NULL;

const char* CURRENT_PATCH_NAME = NULL;

// Critical section object to make patching thread safe. Could be important
// if a user attempts to execute multiple separate scripts at once.
CRITICAL_SECTION PARAM_PATCHER_LOCK;

// Create a new named patch with the given name.
// If a patch already exists under this name, will return a null pointer.
named_patch* CParamUtils_Internal_TryCreateNamedPatch(const char* name, BOOL debug)
{
	named_patch* curr = NAMED_PATCH_LL_HEAD, * prev = NULL;
	for (; curr != NULL && strcmp(curr->name, name); curr = (prev = curr)->next);

	if (curr == NULL)
	{
		named_patch* patch = MALLOC(sizeof(named_patch));
		patch->next = NAMED_PATCH_LL_HEAD;
		patch->name = STRDUP(name);
		patch->uid = ++NAMED_PATCH_UID_CTR;
		patch->diffs_num = 0;
		patch->diffs_cap = NAMED_PATCH_INIT_DIFF_CAP;
		patch->diff_stacks = MALLOC(sizeof(mem_diff_stack*) * NAMED_PATCH_INIT_DIFF_CAP);
		patch->diffs = MALLOC(sizeof(void*) * NAMED_PATCH_INIT_DIFF_CAP);

		NAMED_PATCH_LL_HEAD = patch;
		CURRENT_PATCH_NAME = STRDUP(name);
		return patch;
	}
	else if (debug)
	{
		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "print(\"CParamUtils Error: Tried to create new patch under existing name \\\"%s\\\"\")", name);
		CELUA_ExecuteFunctionAsync(buffer, 0);
	}
	return NULL;
}

// Signify the current named patch is complete. Release the internal param patcher lock, and
// prevents any future patches from being made under this name until the script is disabled.
extern void CParamUtils_Internal_FinalizeNamedPatch()
{
	if (CURRENT_PATCH_NAME != NULL)
	{
		FREE(CURRENT_PATCH_NAME);
		CURRENT_PATCH_NAME = NULL;
	}
	LeaveCriticalSection(&PARAM_PATCHER_LOCK);
}

// Declare a new named param patch. This acquires a critical section, so CParamUtils_Internal_FinalizeNamedPatch
// MUST be called after all desired patches have been applied to release it.
void CParamUtils_Internal_BeginNamedPatch(const char* patch_name, BOOL debug)
{
	// Don't let an idiot pass a null pointer as a patch name
	if (patch_name == NULL) patch_name = "";

	EnterCriticalSection(&PARAM_PATCHER_LOCK);

	// Same patch is still in progress and this call is superfluous, release and do nothing
	if (CURRENT_PATCH_NAME != NULL && !strcmp(patch_name, CURRENT_PATCH_NAME))
		LeaveCriticalSection(&PARAM_PATCHER_LOCK);
	else
	{
		// Different patch was in progress on this thread and was not Finalized(); do it for the user
		if (CURRENT_PATCH_NAME != NULL) CParamUtils_Internal_FinalizeNamedPatch();
		CParamUtils_Internal_TryCreateNamedPatch(patch_name, debug);
	}
}

// If a previous call to BeginNamedPatch specified a valid name, returns the current patch object
// instance. Otherwise, returns a null pointer.
extern void* CParamUtils_Internal_GetPatchIns(BOOL debug)
{
	if (CURRENT_PATCH_NAME == NULL && debug)
		CELUA_ExecuteFunctionAsync("print(\"CParamUtils Error: Tried to patch params without an active named patch in progress\")", 0);
	return (CURRENT_PATCH_NAME == NULL) ? NULL : NAMED_PATCH_LL_HEAD;
}

// Begin a memory patch, and return a pointer to the given param row's data.
void* CParamUtils_Internal_BeginRowPatch(int32_t param_index, int32_t row_index)
{
	return mds_push_begin(&MEM_DIFFS[param_index][row_index]);
}

// Call immediately after having called BeginPatch and having modified the returned param row.
extern void CParamUtils_Internal_FinalizeRowPatch(named_patch* patch, int32_t param_index, int32_t row_index)
{
	mem_diff* diff = mds_push_end(&MEM_DIFFS[param_index][row_index], patch->uid);
	if (diff != NULL)
	{
		if (patch->diffs_num == patch->diffs_cap)
		{
			patch->diffs_cap *= 2;
			patch->diffs = REALLOC(patch->diffs, sizeof(mem_diff*) * patch->diffs_cap);
			patch->diff_stacks = REALLOC(patch->diff_stacks, sizeof(mem_diff_stack*) * patch->diffs_cap);
		}
		patch->diffs[patch->diffs_num] = diff;
		patch->diff_stacks[patch->diffs_num++] = &MEM_DIFFS[param_index][row_index];
	}
}

// Attempts to restore a named param patch. Returns FALSE if the patch was not found.
extern BOOL CParamUtils_Internal_RestorePatch(const char* name, BOOL debug)
{
	// Don't let an idiot pass a null pointer as a patch name
	if (name == NULL) name = "";

	EnterCriticalSection(&PARAM_PATCHER_LOCK);

	named_patch* curr = NAMED_PATCH_LL_HEAD, * prev = NULL;
	for (; curr != NULL && strcmp(curr->name, name); curr = (prev = curr)->next);

	if (curr != NULL)
	{
		for (int i = 0; i < curr->diffs_num; i++)
			mds_restore(curr->diff_stacks[i], curr->diffs[i]);

		FREE(curr->diffs);
		FREE(curr->diff_stacks);
		FREE(curr->name);

		if (prev == NULL) NAMED_PATCH_LL_HEAD = curr->next;
		else prev->next = curr->next;

		FREE(curr);
	}
	else if (debug)
	{
		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "print(\"CParamUtils Error: Patch \\\"%s\\\" cannot be restored, as it doesn't exist\")", name);
		CELUA_ExecuteFunctionAsync(buffer, 0);
	}
	// User is trying to restore the same patch they are currently working on
	// Remove it without leaving the critical section, to account for their call to End()
	if (CURRENT_PATCH_NAME != NULL && !strcmp(CURRENT_PATCH_NAME, name))
	{
		FREE(CURRENT_PATCH_NAME);
		CURRENT_PATCH_NAME = NULL;
	}

	LeaveCriticalSection(&PARAM_PATCHER_LOCK);
	return curr != NULL;
}

typedef struct _paramdef_metadata
{
	uint32_t name_offset; // If set to 0, means this is an array terminator
	uint32_t field_bitmask_offset;
} paramdef_metadata;

// Pointer to an array of paramdef metadata structs, set by an external lua script.
// The param patcher will still work without this data, but expect undefined behavior
// when attempting to resolve patches to the same field out-of-order.
extern paramdef_metadata PARAMDEF_META[1];

// Search the paramdef metadata array for a field mask for the given paramdef.
// return NULL if not found.
field_bitmask* get_field_mask(const char* def_name)
{
	if (PARAMDEF_META == 1) return NULL;

	for (int i = 0; PARAMDEF_META[i].name_offset != 0; i++)
	{
		if (!strcmp((char*)PARAMDEF_META + PARAMDEF_META[i].name_offset, def_name))
			return (field_bitmask*)((char*)PARAMDEF_META + PARAMDEF_META[i].field_bitmask_offset);
	}
	return NULL;
}

// Parse game param list & build data structures
void CParamUtils_Init()
{
	if (CSRegulationManager == 0 || PARAM_MAP != 0) return;

	InitializeCriticalSection(&PARAM_PATCHER_LOCK);

	uint64_t num_params = CSRegulationManager->param_list_end - CSRegulationManager->param_list_begin;
	PARAM_MAP = hm_create(2 * num_params, (hash_fun)wstr_hash, (eq_fun)wstr_eq, FALSE, FALSE);
	MEM_DIFFS = CALLOC(num_params, sizeof(mem_diff_stack*));

	for (size_t i = 0; i < num_params; i++)
	{
		ParamResCap* res_cap = CSRegulationManager->param_list_begin[i];
		ParamTable* tbl = res_cap->param_header->param_table;

		param_info* pinfo = MALLOC(sizeof(param_info));
		pinfo->name = dlw_c_str(&res_cap->param_name);
		pinfo->index = i;
		pinfo->row_size = get_param_size(tbl);
		pinfo->type = get_param_type(tbl);
		pinfo->table = tbl;

		pinfo->row_id_map = hm_create(2 * (size_t)tbl->num_rows, (hash_fun)u64_hash, (eq_fun)u64_eq, FALSE, FALSE);
		if (pinfo->row_size > 0)
		{
			field_bitmask* fbm = get_field_mask(pinfo->type);
			MEM_DIFFS[i] = MALLOC(sizeof(mem_diff_stack) * tbl->num_rows);
			for (int j = 0; j < tbl->num_rows; j++)
			{
				mds_init(&MEM_DIFFS[i][j], get_row_data(tbl, j), pinfo->row_size, fbm);
				hm_set(pinfo->row_id_map, (void*)tbl->rows[j].row_id, j);
			}
		}

		hm_set(PARAM_MAP, pinfo->name, pinfo);
	}
	// Enable memory (de)allocation logging after setup is complete
	MEM_LOG_ENABLED = TRUE;
}
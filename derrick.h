/**
 * @file        derrick.c
 * @author      Mathieu Allory
 * @date        January 2018
 * @brief       Inspector Derrick's deep file search library
 *
 * @details     This header provides the definitions required to use derrick DFS
 *
 * @license
 */

// Status codes returned by functions
#define DERRICK_OK              0
#define DERRICK_NO_INPUT        1
#define DERRICK_TOO_LONG        2
#define DERRICK_PATH_NOT_FOUND  3
#define DERRICK_ERROR           4

// Some compiler dependent stuffs
#ifdef _MSC_VER
# define BYTEP char
#else
# define BYTEP void
#endif

// Definition of the callback called to determine whether
// a file shall be excluded from search
// return 0 -> keep the file
// return != 0 -> exclude the file
typedef int  (*derrick_cb_exclude_t)(void* context, const char* file);

// Callback called when a match is found
typedef void (*derrick_cb_found_t)  (void* context, const char* in, const char* what);

// Structure containing the parameters for some function calls
struct Derrick_Parameters_s
{
    derrick_cb_exclude_t cb_exclude;
    derrick_cb_found_t cd_found;
    void* ctx_exclude;
    void* ctx_found;
    int param_case_sensitive; // Not used, reserved
};
typedef struct Derrick_Parameters_s * Derrick_Parameters;

// Structure containing the index for index-based funtions
struct DerrickIndex_s
{
    size_t number_of_entries;
    struct Derrick_EntryHeader_s* index;
};
typedef struct DerrickIndex_s * DerrickIndex;

// This structure is for internal use
struct Derrick_EntryHeader_s
{
    size_t size;
    char name[];
};

/**
 * @brief derrick_index_build
 * @param io_index
 * @param i_path
 * @return
 */
int derrick_index_build(DerrickIndex* io_index, const char* i_path);
void derrick_index_list(DerrickIndex i_index);
void derrick_index_search(DerrickIndex i_index, const char* i_searchfor, int i_case_sensitive);

int derrick_count_files(const char* i_searchin);
int derrick_deep_search(const char* i_searchfor, const char* i_searchin, int i_case_sensitive, Derrick_Parameters io_cb);

/**
 * @file        derrick.h
 * @author      Mathieu Allory
 * @date        February 2018
 * @brief       Derrick DFS: deep file search and indexing library
 * @ref         https://github.com/thew44/derrick
 *
 * @details     This header provides the definitions required to use Derrick DFS
 *
 * @license     MIT License
 */

// Status codes returned by functions
#define DERRICK_OK              0
#define DERRICK_NO_INPUT        -2
#define DERRICK_TOO_LONG        -3
#define DERRICK_PATH_NOT_FOUND  -4
#define DERRICK_ERROR           -1

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
 * @brief Initialize a parameter structure with neutral values
 * @param io_cb the structure to initialize
 */
void derrick_init_parameters(Derrick_Parameters io_cb);

/**
 * @brief Build an in-memory index from the files contained in a given directory i_path
 * @param io_index Address of pointer where the structure will be created
 * @param i_path Path to index
 * @return DERRICK_OK if no error
 */
int derrick_index_build(DerrickIndex* io_index, const char* i_path);

/**
 * @brief list on standard output the files contained in a given index
 * @param i_index the index previously built with derrick_index_build
 */
void derrick_index_list(DerrickIndex i_index);

/**
 * @brief search for the file(s) containing a given string within the given index.
 * Indexed search is very fast but consumes a lot of memory and not synchronized with hard drive.
 * @param i_index the index previously built with derrick_index_build
 * @param i_searchfor the string the look for
 * @param io_cb the callbacks and parameters, see definition
 */
void derrick_index_search(DerrickIndex i_index, const char* i_searchfor, Derrick_Parameters io_cb);

/**
 * @brief count the files that would be searched, the same way as derrick_deep_search would do
 * but without actually looking inside the file
 * @param i_searchin the root path to search in
 * @return the total number of files that matches the criteria of io_cb->cb_exclude, or -1 in case of error
 */
int derrick_count_files(const char* i_searchin, Derrick_Parameters io_cb);

/**
 * @brief search for the file(s) containing a given string i_searchfor in directory i_searchin by
 * examining all the files on the disk. The actual content of every file is scanned.
 * @param i_searchfor the string to look for
 * @param i_searchin the root path to search in
 * @param io_cb the callbacks and parameters, see definition
 * @return DERRICK_OK if no error
 */
int derrick_deep_search(const char* i_searchfor, const char* i_searchin, Derrick_Parameters io_cb);

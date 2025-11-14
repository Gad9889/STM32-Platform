
#include "database.h"
#include "DbSetFunctions.h"


/* =============================== Global Variables =============================== */
static database_t* pMainDB = NULL;

/* ========================== Function Definitions ============================ */
/**
 * @brief Initialize the database
 * @retval Pointer to the initialized database
 * @note This function is used to initialize the database
 */
database_t* db_Init()
{
   pMainDB = db_AllocateMemory();
   DbSetFunctionsInit();
   return pMainDB;
}

/**
 * @brief Allocate memory for the database
 * @retval Pointer to the allocated database
 */
database_t* db_AllocateMemory() 
{
database_t* db = (database_t*)calloc(1, sizeof(database_t));
if (db == NULL) {return NULL;}  // Handle allocation failure

//Initialize Pedal node
db->pedal_node = (pedal_node_t*)calloc(1, sizeof(pedal_node_t));
if (db->pedal_node == NULL) {free(db);return NULL;} // Handle allocation failure

//Initialize Sub node
db->sub_node = (sub_node_t*)calloc(1, sizeof(sub_node_t));
if (db->sub_node == NULL) {free(db->pedal_node);free(db);return NULL;}  // Handle allocation failure

//Initialize VCU node
db->vcu_node = (vcu_node_t*)calloc(1, sizeof(vcu_node_t));
if (db->vcu_node == NULL) {free(db->sub_node);free(db->pedal_node);free(db);return NULL;} // Handle allocation failure

//Initialize Dashboard node
db->dashboard_node = (dashboard_node_t*)calloc(1, sizeof(dashboard_node_t));
if (db->dashboard_node == NULL) {free(db->vcu_node);free(db->sub_node);free(db->pedal_node);free(db);return NULL;} // Handle allocation failure

return db;
}

/**
 * @brief Free memory allocated for the database
 * @param db_ptr Pointer to the database to be freed
 */
void db_FreeMemory(database_t* db_ptr){
    
    if (db_ptr == NULL) {
        return; // NULL pointer protection
    }
    
    //free memory of db members from the heap
    free(db_ptr->pedal_node);
    free(db_ptr->dashboard_node);
    free(db_ptr->sub_node);
    free(db_ptr->vcu_node);

    //free db
    free(db_ptr);
}

/**
 * @brief Get function for the database pointer
 * @retval Pointer to the database
 * @note This function is used to get the pointer to the database
 */
database_t* db_GetDBPointer(){
    return pMainDB;
}

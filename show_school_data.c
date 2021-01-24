#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* We need to include this to get access to JSON types and functions */
#include <json-c/json.h>


/* Macro to help simplify fatal error checking and handling */
#define FAIL_ON_VAL(result, val, msg)            \
    if ((result) == (val)) {                     \
        fprintf(stderr, "Error: %s\n", msg);     \
        exit(EXIT_FAILURE);                      \
        }


/* Function: get_file_contents 
 * Utility function to get entire contents of specified file as a single string
 * Arguments:
 *     filename - name of file whose contents should be returned.
 * Returns:
 *     contents of file as a string, if successful
 *     NULL if any error occurs 
 */
static char *get_file_contents(char *filename) {
    /* Get the file's size */
    struct stat file_info;
    if (stat(filename, &file_info) == -1)
        return NULL;

    /* Make a buffer large enough to fit the file's contents */
    char *buffer = (char *) malloc(file_info.st_size);
    if (buffer == NULL)
        return NULL;

    /* Open the file and read it all into the buffer */
    FILE *fp = fopen(filename,"r");
    if (fp == NULL) {
        free(buffer);
        return NULL;
        }

    if (fread(buffer, file_info.st_size, 1, fp) != 1) {
        free(buffer);
        fclose(fp);
        return NULL;
        }

    /* Close the file, and return its contents */
    fclose(fp);
    return buffer;
    }


/* Function: get_string_element 
 * Utility function to get an object's child string element.
 * Arguments:
 *     obj  - a JSON object
 *     name - name of a child element that should have a string value
 * Returns:
 *     text of the child element as a string, if successful
 *     NULL if any error occurs (child doesn't exist or isn't a string)
 */
static const char *get_string_element(struct json_object *obj, char *name) {
    /* Get the child element */
    struct json_object *element;
    if (!json_object_object_get_ex(obj, name, &element))
        return NULL;

    /* Make sure it's actually a string */
    if (json_object_get_type(element) != json_type_string) 
        return NULL;

    /* Return it's value as a string */
    return json_object_get_string(element);
    }


/* Function: print_professor
 * Utility function to print a professor's information
 * Arguments:
 *     person - a JSON object representing a professor
 * Note: prints to stdout as a side effect.  The format is:
 *     <name>
 *         Email: <email>
 *         Office: <building> <room>
 *         Teaches:
 *             <course1>
 *             <course2>
 *             ...
 * Note: elements are skipped if not present.  For example, if a professor's
 * record is missing their email, it will not print the email line, but will
 * keep trying to print the rest.  The exception is the name.  If the name 
 * isn't present, the rest doesn't make sense to print (i.e. the whole element
 * passed might be invalid if there's no name).
 */
static void print_professor(struct json_object *person) {
    /* Get and print the name */
    const char *name  = get_string_element(person, "name");
    if (name == NULL) 
        return;

    printf ("    %s\n", name);

    /* Get and print the email */
    const char *email = get_string_element(person, "email");
    if (email != NULL) 
        printf("        Email: %s\n", email);
    
    /* Get and print the office information */
    struct json_object *office;
    if (json_object_object_get_ex(person, "office", &office)) {
        /* Get the building and room */
        const char *building  = get_string_element(office, "building");
        const char *room      = get_string_element(office, "room");

        /* If both are present, print the office information */
        if (building != NULL && room != NULL) 
            printf("        Office: %s %s\n", building, room);
        }

    /* Get and print the courses they teach */
    printf("        Teaches:\n");
    
    /* Get the courses_taught object.  If it's present, loop over the courses */
    struct json_object *courses;
    if (json_object_object_get_ex(person, "courses_taught", &courses)) {
        /* For each course... */
        int n_courses = json_object_array_length(courses);
        for (int i = 0; i < n_courses; i++) {
            /* Get the course object */
            struct json_object *course;
            course = json_object_array_get_idx(courses, i);
            if (course == NULL) 
                return;

            /* Get the text out of the course object and print it */
            const char *course_name = json_object_get_string(course);
            if (course_name != NULL) 
                printf("            %s\n", course_name);
            }
        }
    }


int main() {
    /* Get the contents of "compsci.json" */
    char *file_contents = get_file_contents("compsci.json");
    FAIL_ON_VAL(file_contents, NULL, "Error reading compsci.json");

    /* Convert it to a JSON object */
    struct json_object *data = json_tokener_parse(file_contents);
    FAIL_ON_VAL(data, NULL, "Invalid JSON data");

    /* Clean up the memory used to store the file contents */
    free(file_contents);

    /* Get the school and department from the JSON object, and print them */
    const char *school  = get_string_element(data, "School");
    FAIL_ON_VAL(school, NULL, "Error reading element 'School'\n");
    
    const char *department = get_string_element(data, "Department");
    FAIL_ON_VAL(department, NULL, "Error reading element 'Department'\n");

    printf("%s: %s\n", school, department);

    /* Get the faculty list from the JSON object.  If successful... */
    struct json_object *faculty;
    if (json_object_object_get_ex(data, "Faculty", &faculty)) {
        /* Loop over the items in the faculty list */
        int n_faculty = json_object_array_length(faculty);

        for (int i = 0; i < n_faculty; i++) {
            /* Get the faculty member's object */
            struct json_object *professor;
            professor = json_object_array_get_idx(faculty, i);

            /* Print it (if no error happened) */
            if (professor != NULL)
                print_professor(professor);
            }
        }


    /* Free up the data structure */
    json_object_put(data);
        
    exit(EXIT_SUCCESS);
    }

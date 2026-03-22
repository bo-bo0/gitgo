#define _CRT_SECURE_NO_WARNINGS

#include <stddef.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SHORT_TITLE_LEN 10
#define SHORT_DESCRIPTION_LEN 20

typedef enum  
{
    SUCCESS,
    TAG_NAME_NOT_FOUND,
    COMMIT_TITLE_NOT_FOUND,
    GIT_ADD,
    GIT_COMMIT,
    GIT_TAG_CREATE,
    GIT_TAG_PUSH,
    GIT_PUSH,
    MEM_ALLOC_FAIL
} GitgoExitCode;


typedef struct 
{
   const char* commit_title;
   const char* commit_description;
   const char* tag_name; 
} InputData;

void short_message_handle();
void input_to_data(int argc, const char** argv, InputData* data);
int search_in(const char* target, const char** strings, int size); 
void gitgo_exit(GitgoExitCode code); 
void git_execute(const char* commit_command, const char* tag_create_command);
void* s_malloc(size_t size);

int main(int argc, const char** argv) 
{
    InputData data;
    input_to_data(argc, argv, &data);

    char commit_command[1024];
    char tag_create_command[256];

    if (data.commit_description) 
    {
        snprintf
        (
            commit_command,
            sizeof(commit_command),
            "git commit -m \"%s\" -m \"%s\"",
            data.commit_title, data.commit_description
        );
    }

    else 
    {
        snprintf
        (
            commit_command,
            sizeof(commit_command),
            "git commit -m \"%s\"",
            data.commit_title
        );
    }

    if (strlen(data.commit_title) < SHORT_TITLE_LEN) 
    { short_message_handle(); }

    else if (data.commit_description) 
    {
        if (strlen(data.commit_description) < SHORT_DESCRIPTION_LEN) 
        { short_message_handle(); }
    }

    if (data.tag_name) 
    {   
        snprintf
        (
            tag_create_command,
            sizeof(tag_create_command),
            "git tag %s",
            data.tag_name
        );
    }

    git_execute(commit_command, data.tag_name != NULL ? tag_create_command : NULL);

    return 0;
}

void input_to_data(int argc, const char** argv, InputData* data)
{
    const char* tag_name = NULL;
    const char* commit_title = NULL;
    const char* commit_description = NULL;
    
    int tag_index = search_in("--tag", argv, argc);
    if (tag_index > -1) 
    {
        if (tag_index + 1 >= argc) 
        { gitgo_exit(TAG_NAME_NOT_FOUND); }

        else 
        { tag_name = argv[tag_index + 1]; }
    }

    int j = 0;
    for (int i = 1; i < argc; i++) 
    {
        if ((i != tag_index) && (i != tag_index + 1)) 
        {
            if (j == 0) 
            {
                commit_title = argv[i];
                j++;
            }

            else 
            {
                commit_description = argv[i];
                break;
            }
        }
    }

    if (commit_title == NULL) 
    { gitgo_exit(COMMIT_TITLE_NOT_FOUND); }

    data->tag_name = tag_name;
    data->commit_title = commit_title;
    data->commit_description = commit_description;
}

int search_in(const char* target, const char** strings, int size) 
{
    for (int i = 0; i < size; i++) 
    {
        if (strcmp(strings[i], target) == 0) 
        { return i; }
    }

    return -1;
}

void short_message_handle() 
{
    char c;

    printf("Warning: a very short commit message was detected, ");
    printf("are you sure you want to proceed?[y/n]: ");
    scanf(" %c", &c);

    c = tolower(c);

    if (c != 'y') 
    {
        puts("Operation aborted");
        gitgo_exit(SUCCESS);
    } 

    puts("");
}

void gitgo_exit(GitgoExitCode code) 
{
    const char* error_message;

    switch (code) 
    {
        case TAG_NAME_NOT_FOUND:
            error_message = "tag flag was activated but no tag name was found";
            break;

        case COMMIT_TITLE_NOT_FOUND:
            error_message = "a commit needs to have a title [gitgo \"title\"]";
            break;

        case GIT_ADD:
            error_message = "git add failed (git error)";
            break;

        case GIT_COMMIT:
            error_message = "git commit failed (git error)";
            break;

        case GIT_TAG_CREATE:
            error_message = "git tag failed (git error)";
            break;

        case GIT_TAG_PUSH:
            error_message = "git push origin --tags failed (git error)";
            break;

        case GIT_PUSH:
            error_message = "git push failed (git error)";
            break;

        case MEM_ALLOC_FAIL:
            error_message = "memory allocation failed";
            break;

        default:
            error_message = NULL;
            break;
    }

    if (code) 
    {
        if (error_message == NULL) 
        { error_message = "uknown error"; }

        fprintf(stderr, "Error: %s", error_message);
        exit(EXIT_FAILURE);
    }

    else 
    { exit(EXIT_SUCCESS); }
}

void git_execute(const char* commit_command, const char* tag_create_command) 
{
    int g_adderr = system("git add .");

    if (g_adderr != 0) 
    { gitgo_exit(GIT_ADD); }

    int g_commiterr = system(commit_command);

    if (g_commiterr != 0) 
    { gitgo_exit(GIT_COMMIT); }

    int g_tagcreateerr = 0;
    if (tag_create_command) 
    { 
        g_tagcreateerr = system(tag_create_command);

        if (g_tagcreateerr != 0) 
        { gitgo_exit(GIT_TAG_CREATE); }

        int g_tagpusherr = system("git push origin --tags");

        if (g_tagpusherr != 0) 
        { gitgo_exit(GIT_TAG_PUSH); }
    }

    int g_pusherr = system("git push");

    if (g_pusherr != 0) 
    { gitgo_exit(GIT_PUSH); }
}

void* s_malloc(size_t size) 
{
    void* p = malloc(size);

    if (!p) 
    { gitgo_exit(MEM_ALLOC_FAIL); }

    return p;
}
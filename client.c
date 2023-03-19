#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char* extract_cookie(char* response) {
    char* str = strstr(response, "connect.sid=");

    char* cookie = strtok(str, ";");

    
    return cookie;
}

char* extract_jwt(char* response) {
    char* str = strstr(response, "token\":\"");
    if(str == NULL)
        return str;

    char* token = strtok(str, "\"");
    token = strtok(NULL, "\"");
    token = strtok(NULL, "\"");

    return token;
}

char* get_books_response(char* response) {
    char *token = NULL;
    
    char* str = strstr(response, "error");
    if (str) {
        token = strtok(str, "\"");
        token = strtok(NULL, "\"");
        token = strtok(NULL, "\"");
    } else {
        token = strstr(response, "[");
    }
        
    return token;
}

char* extract_error(char* response) {
    char *token = NULL;
    
    char* str = strstr(response, "error");
    if (str) {
        token = strtok(str, "\"");
        token = strtok(NULL, "\"");
        token = strtok(NULL, "\"");
    } else {
        char*aux = strtok(response, "\n");
        token = strtok(aux, " ");
        token = strtok(NULL, "\n");
    }
        
    return token;
}

int isNumber(char s[])
{
    if(strlen(s) > 1 && s[0] == '0')
        return 0;

    for (int i = 0; s[i]!= '\0'; i++){
        if (strchr("0123456789", s[i]) == NULL)
            return 0;
    }
    return 1;
}

char* book_json(char* title, char* author, char* genre, char* page_count, char* publisher) {
    char* book_info = malloc(LINELEN);

    sprintf(book_info, 
    "{\n\t\"title\": \"%s\",\n\t\"author\": \"%s\",\n\t\"genre\": \"%s\",\n\t\"page_count\": %s,\n\t\"publisher\": \"%s\"\n}",
    title, author, genre, page_count, publisher);

    return book_info;
}

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    char *command = malloc(LINELEN);
    char **cookies = calloc(1, sizeof(char *));
    char* jwt = calloc(1, LINELEN);

    while (1)
    {
        scanf("%s", command);
        sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

        if (strcmp(command, "register") == 0) {
            char username[ENTRY], password[ENTRY];
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);

            char** form_data = calloc(2, sizeof(char *));
            form_data[0] = calloc(1, LINELEN);
            sprintf(form_data[0], "{\n\t\"username\": \"%s\",\n\t\"password\": \"%s\"\n}", username, password);
            form_data[1] = NULL;

            message = compute_post_request("localhost", "/api/v1/tema/auth/register", "application/json", form_data, NULL, 0);
            puts(message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            puts(extract_error(response));
        }

        if (strcmp(command, "login") == 0)
        {
            char username[ENTRY], password[ENTRY];
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);

            char** form_data = calloc(2, sizeof(char *));
            form_data[0] = calloc(1, LINELEN);
            sprintf(form_data[0], "{\n\t\"username\": \"%s\",\n\t\"password\": \"%s\"\n}", username, password);
            form_data[1] = NULL;


            message = compute_post_request("localhost", "/api/v1/tema/auth/login", "application/json", form_data, NULL, 0);
            puts(message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            cookies[0] = extract_cookie(response);
            puts(extract_error(response));
        }

        if (strcmp(command, "enter_library") == 0) {
            message = compute_get_request("localhost", "/api/v1/tema/library/access", NULL, cookies, 1);
            puts(message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char* aux = calloc(1, LINELEN);
            strcpy(aux, extract_jwt(response));
            if(aux){
                sprintf(jwt, "Authorization: Bearer %s", aux);
            }
            puts(extract_error(response));
        }

        if(strcmp(command, "get_books") == 0) {
            message = compute_get_request("localhost", "/api/v1/tema/library/books", jwt, NULL, 0);
            puts(message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            puts(get_books_response(response));
        }

        if(strcmp(command, "get_book") == 0) {
            int id;
            printf("id=");
            scanf("%d", &id);

            char* aux = malloc(100);
            sprintf(aux, "/api/v1/tema/library/books/%d", id);

            message = compute_get_request("localhost", aux, jwt, NULL, 0);
            puts(message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            puts(get_books_response(response));
        }

        if(strcmp(command, "add_book") == 0) {
            char title[ENTRY], author[ENTRY], genre[ENTRY], publisher[ENTRY], page_count[ENTRY];

            printf("title=");
            scanf("%s", title);
            printf("author=");
            scanf("%s", author);
            printf("genre=");
            scanf("%s", genre);
            printf("publisher=");
            scanf("%s", publisher);
            printf("page_count=");
            scanf("%s", page_count);

            if(isNumber(page_count)){
                char** form_data = calloc(2, sizeof(char *));
                form_data[0] = malloc(LINELEN);
                sprintf(form_data[0], "%s", book_json(title, author, genre, page_count, publisher));
                form_data[1] = malloc(LINELEN);
                sprintf(form_data[1], "%s", jwt);

                message = compute_post_request("localhost", "/api/v1/tema/library/books", "application/json", form_data, NULL, 0);
                puts(message);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                puts(extract_error(response));
            }else printf("Informatiile introduse sunt incompatibile");

        }

        if (strcmp(command, "delete_book") == 0) {
            int id;
            printf("id=");
            scanf("%d", &id);

            char* aux = malloc(100);
            sprintf(aux, "/api/v1/tema/library/books/%d", id);

            message = compute_delete_request("localhost", aux, jwt);
            puts(message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            puts(extract_error(response));

        }

        if (strcmp(command, "logout") == 0) {
            message = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout", NULL, cookies, 1);
            puts(message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            puts(extract_error(response));
        }

        if (strcmp(command, "exit") == 0) {
            close(sockfd);
            free(message);
            free(response);
            exit(0);
        }
    }

    
    return 0;
}

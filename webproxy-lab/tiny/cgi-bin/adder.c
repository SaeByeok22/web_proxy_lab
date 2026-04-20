/*
 * adder.c - a tiny CGI program that adds two numbers.
 */
#include "csapp.h"

static int value_from_arg(char *arg)
{
  char *eq = strchr(arg, '=');

  if (eq)
    return atoi(eq + 1);
  return atoi(arg);
}

int main(void)
{
  char *query;
  char query_copy[MAXLINE];
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  char *amp;
  int n1 = 0, n2 = 0;

  query = getenv("QUERY_STRING");
  if (query == NULL)
    query = "";

  snprintf(query_copy, sizeof(query_copy), "%s", query);
  amp = strchr(query_copy, '&');
  if (amp)
  {
    *amp = '\0';
    snprintf(arg1, sizeof(arg1), "%s", query_copy);
    snprintf(arg2, sizeof(arg2), "%s", amp + 1);
    n1 = value_from_arg(arg1);
    n2 = value_from_arg(arg2);
  }

  snprintf(content, sizeof(content),
           "QUERY_STRING=%s\r\n<p>"
           "Welcome to add.com: "
           "THE Internet addition portal.\r\n<p>"
           "The answer is: %d + %d = %d\r\n<p>"
           "Thanks for visiting!\r\n",
           query, n1, n2, n1 + n2);

  printf("Content-type: text/html\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("\r\n");
  printf("%s", content);
  fflush(stdout);

  exit(0);
}

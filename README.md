# web_server_cpp

A simple web (HTTP) server, which is able to respond to requests from the previous web client, as well as requests from
commercial web browsers (such as Chrome, Firefox, Safari, etc.). 

However, it only respond successfully to GET requests with error codes if necessary. It takes command line argument, which will be the port number that it should listen on.

Following information will be displayed after the operations:

- Connection
- Date
- Last-Modified
- Content-Length
- Content-Type


## Desciption
The server will accept connections on that port number, receive an HTTP request, send a response, and then close the connection. The response should use the appropriate “Connection:” header field to inform the client that it will be closing the connection.

The server does not handle multiple clients at the same time, but after finishing with one client, it loops to accept and handle another
client. If the request is a GET request, then it retrieves the document indicated by the path component of the request line. 

If the request is any method other than GET, it sends a response with the 501 - Not Implemented status code. It will still include the “Connection:” header field when sending the response. The server has a directory called web_root for testing whether the request and retreive is done properly.

Also, any request path that indicates a directory rather than a file will return the index.html file in that directory (if one exists, and 404 Not Found if there is no index.html there). For example, path / returns /index.html and /foo or /foo/ returns /foo/index.html.

Lastly, if a /../ directory is included anywhere in the path, it will return 400 Bad Request.

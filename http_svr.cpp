/**
 * CPSC 5510
 * Computer Networking
 * Heetae Yang
 * Project 2: Web Server
 * 5/19/2020
 */

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <vector>

using namespace std;

int socketSetup(int);

void respondClient(int);

void getHeader(int, string &);

void parseRequestHeader(const string &, string &);

bool checkFileExtension(const string &, string &);

string errorCode(string);

string statusCode(const string &, string &, string &);

string generateHeader(const string &, const string &, string);

void getSucess(const string &, string &, string &);

int sendHeader(int, char *, int);

void sendFile(int, string);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    int socket = socketSetup(atoi(argv[1]));
    cout << "Server listening on the port " << argv[1] << endl;
    respondClient(socket);

    close(socket);
    return 0;
}

/**
 * Sets up the socket for communication
 *
 * @param Port number
 * @return Created socket
 */
int socketSetup(int portno) {
    int sock;
    struct sockaddr_in svr_addr;

    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(portno);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "ERROR opening socket";
        exit(1);
    }

    int retval = ::bind(sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr));
    if (retval < 0) {
        cerr << "ERROR on binding to the port.";
        exit(1);
    }

    retval = listen(sock, 5);
    if (retval < 0) {
        cerr << "ERROR Listen" << endl;
        exit(1);
    }

    return sock;
}

/**
 * Takes the socket and do the all the necessary functions including
 * getting, parsing, and generating header.
 *
 * Then it sends the message back to the client. If the file is requested and
 * passes the error checking, it will send the file.
 *
 * @param socket
 */
void respondClient(int socket) {
    string header, parsedHeader, path, contentType;
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    while (true) {
        int cli_sock = accept(socket, (struct sockaddr *) &cli_addr, &cli_len);
        if (cli_sock < 0) {
            cerr << "ERROR on accept." << endl;
        } else {
            getHeader(cli_sock, header);
            parseRequestHeader(header, parsedHeader);

            string statCode = statusCode(parsedHeader, path, contentType);
            cout << "Requested path: " << path << endl;
            cout << endl;
            string response = generateHeader(path, statCode, contentType);

            sendHeader(cli_sock, (char *) response.c_str(), response.size());
            if (statCode == "200")
                sendFile(cli_sock, path);

            header.clear();
        }
        close(cli_sock);
    }
}

/**
 * Gets header from the client and returns the header reference.
 *
 * @param socket
 * @param header string; make as reference.
 */
void getHeader(int socket, string &header) {
    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    int i = 0;
    int retVal;

    retVal = recv(socket, buffer, sizeof(buffer), 0);
    if (retVal < 0) {
        cerr << "ERROR failed to receive string length";
        exit(1);
    }

    if (retVal <= 0) {
        cerr << "ERROR failed to receive valid string length from the client"
             << endl;
        exit(1);
    }

    while (i < retVal) {
        header += buffer[i];
        i++;
    }
}

/**
 * Parses the first line and puts the result into the parsedHeader.
 *
 * @param header
 * @param parsedHeader parses header and make as a reference
 */
void parseRequestHeader(const string &header, string &parsedHeader) {
    string::size_type firstLine;
    firstLine = header.find("\r\n");
    if (firstLine == string::npos) {
        cerr << "No request is found." << endl;
        exit(1);
    }

    parsedHeader = header.substr(0, firstLine);
}

/**
 * Generates header for client response.
 *
 * @param path
 * @param statusCode
 * @param contentType
 * @return the response
 */
string generateHeader(const string &path, const string &statusCode,
                      string contentType) {
    string response;
    string failedlength;
    response += "HTTP/1.1 ";
    failedlength = "Content-Length: 0\r\n";

    if (statusCode == "200") {
        getSucess(path, response, contentType);
    } else if (statusCode == "400") {
        response += statusCode + " Bad Request\r\n" + failedlength;
    } else if (statusCode == "501") {
        response += statusCode + " Not Implemented\r\n" + failedlength;
    } else if (statusCode == "404") {
        response += statusCode + " Not Found\r\n" + failedlength;
    } else {
        response +=
                statusCode + " Internal Server Error\r\n" + failedlength;
    }

    response += "Connection: closed\r\n\r\n";
    return response;
}

/**
 * Takes in the path and check whether the requested file is supported by the
 * server.
 *
 * @param path of the request
 * @param contentType returns the contentType as reference
 * @return whether path's file is supported or not
 */
bool checkFileExtension(const string &path, string &contentType) {
    size_t ext = path.find('.');
    if (ext == string::npos)
        return false;

    string fileExt = path.substr(ext + 1);
    if (fileExt == "txt") {
        contentType = "text/plain";
        return true;
    } else if (fileExt == "html") {
        contentType = "text/html";
        return true;
    } else if (fileExt == "htm") {
        contentType = "text/htm";
        return true;
    } else if (fileExt == "css") {
        contentType = "text/css";
        return true;
    } else if (fileExt == "jpg" || fileExt == "jpeg") {
        contentType = "image/jpeg";
        return true;
    } else if (fileExt == "png") {
        contentType = "image/png";
        return true;
    } else {
        return false;
    }
}

/**
 * Returns the error code
 *
 * @param code
 * @return error code
 */
string errorCode(string code) {
    if (code == "400") {
        cerr << "400 Bad Request" << endl;
    } else if (code == "501") {
        cerr << "501 Not Implemented" << endl;
    } else if (code == "404") {
        cerr << "404 Not Found" << endl;
    } else if (code == "500") {
        cerr << "500 Internal Server Error" << endl;
    } else {
        cerr << "Unknown error" << endl;
    }
    return code;
}

/**
 * Generates the status of the client request. It will perform error checking
 * on given request type, file path, and content types.
 *
 * If it passes the base error checkings, it runs the rest of lines when there
 * are no errors present.
 *
 * If file extension is not found (file name not specified), adds default return
 * of index.html
 *
 * @param messages is the parsed header
 * @param path from the parsed header
 * @param contentType of the path's file
 * @return the final status code of the request
 */
string statusCode(const string &messages, string &path, string &contentType) {
    string statCode;
    bool curr_error = false;
    size_t space = messages.find(' ');
    string requestType = messages.substr(0, space);

    int p = messages.rfind(' ');
    path = messages.substr(space + 1, p - space - 1);
    string version = messages.substr(messages.rfind(' ') + 1,
                                     messages.length());

    if (requestType != "GET") {
        statCode = errorCode("501");
        curr_error = true;
    }

    if (path.find("/../") != string::npos ||
        path.find("..") != string::npos ||
        path.back() == ('.')) {
        statCode = errorCode("400");
        curr_error = true;
    }

    if (version != "HTTP/1.1") {
        statCode = errorCode("501");
        curr_error = true;
    }

    if (!curr_error) {
        struct stat file_info;
        path = "web_root" + path;

        if (stat(path.c_str(), &file_info) != 0) {
            statCode = errorCode("404");
        } else {
            if (S_ISDIR(file_info.st_mode)) {
                if (path.find('.') == string::npos) {
                    if (path.back() != '/')
                        path += "/";
                    path += "index.html";
                }
            }

            if (!checkFileExtension(path, contentType))
                statCode = "501";
            else
                statCode = "200";
        }
    }
    return statCode;
}

/**
 * Generates the time.
 *
 * @param t
 * @return time as the string
 */
string dt(tm t) {
    char buffer[100];
    strftime(buffer, 100, "%a, %d %h %G %T GMT", &t);
    string dt = buffer;
    return dt;
}

/**
 * When the status is success(200), it will generate information about the
 * request's date, last-modified date, content type and its length.
 *
 * @param path
 * @param response
 * @param contentType
 */
void getSucess(const string &path, string &response, string &contentType) {
    time_t now = time(nullptr);
    tm gmt = *gmtime(&now);
    string date = dt(gmt);

    struct stat file_info;
    if (stat(path.c_str(), &file_info) == -1) {
        cerr << "Failed retrieving file information." << endl;
        return;
    }
    string mdate = dt(*gmtime(&(file_info.st_mtime)));

    string conLength = to_string(file_info.st_size);

    response += "200 OK\r\nContent-Length: " + conLength + "\r\n"
                + "Date: " + date + "\r\n"
                + "Last-Modified: " + mdate + "\r\n"
                + "Content-Type: " + contentType + "\r\n";
}

/**
 * Sends back the generated header message
 *
 * @param socket
 * @param buffer - to read and send data
 * @param length - number of bytes to send.
 * @return error code (0 for success, -1 if an error occurred)
 */
int sendHeader(int socket, char *buffer, int length) {
    while (length > 0) {
        int i = send(socket, buffer, length, 0);
        if (i < 0) {
            cerr << "Send failed" << endl;
            return -1;
        }

        buffer += i;
        length -= i;
    }
    return 0;
}

/**
 * Will read the file from the given path, copy the contents from the file,
 * and send its content.
 *
 * The for loop will copy the buffer with checking
 * 1. It isn't copying stuff outside the buffer size,
 * 2. It isn't copying stuff outside the size of the data.
 *
 * @param socket
 * @param path
 */
void sendFile(int socket, string path) {
    ifstream ifs(path);
    vector<char> data = vector<char>(std::istreambuf_iterator<char>(ifs),
                                     std::istreambuf_iterator<char>());

    int sent = 0;
    int filesize = data.size();
    char buffer[4096];
    int buffsize = (int) sizeof(buffer);
    memset(buffer, 0, sizeof(buffer));

    for (int i = 0; i < buffsize && (i + sent < filesize); i++) {
        buffer[i] = data[i + sent];
    }

    while (sent < filesize) {
        int n = send(socket, &buffer, sizeof(buffer), 0);
        if (n < 0) {
            cerr << "ERROR sending file to socket" << endl;
            exit(1);
        }

        sent += n;
        memset(buffer, 0, sizeof(buffer));
        for (int i = 0; i < buffsize && (i + sent < filesize); i++) {
            buffer[i] = data[i + sent];
        }
    }
}
/**
 * CPSC 5510
 * Computer Networking
 * Heetae Yang
 * Project 1: Web Client - Modified Ver.
 * 5/19/2020
 */

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <cstdio>
#include <sstream>

using namespace std;

void parseUrl(const char *, string &, string &, string &);

int sendmessage(int, char *, int);

int connection(string, string);

void requestSvr(int, string, string);

void recvAll(int);

int main(int argc, char *argv[]) {
    if (argc < 1) {
        fprintf(stderr, "Input Invalid. Usage: %s hostname \n", argv[0]);
        exit(0);
    }

    const char *host = argv[1];
    string serverName, portNo, filePath;
    string hostname = host;
    parseUrl(hostname.c_str(), serverName, portNo, filePath);

    int sockfd = connection(portNo, serverName);
    requestSvr(sockfd, serverName, filePath);
    recvAll(sockfd);

    close(sockfd);
    return 0;
}

void parseUrl(const char *url, string &serverName, string &portNo,
              string &filepath) {
    string::size_type colon, slash;
    string urlName = url;

    if (urlName.substr(0, 7) == "http://")
        urlName.erase(0, 7);

    colon = urlName.find(':');
    slash = urlName.find('/');

    // if there is a port number given
    if (colon != string::npos) {
        serverName = urlName.substr(0, colon);
        if (slash != string::npos) {
            portNo = urlName.substr(colon + 1, slash - colon - 1);
            filepath = urlName.substr(slash);
        } else {
            portNo = urlName.substr(colon + 1, urlName.length());
            filepath = "/";
        }

        // if input was given without the port number and has file path
        // e.g. http://pages.cs.wisc.edu/~remzi/OSTEP
    } else if (slash != string::npos) {
        serverName = urlName.substr(0, slash);
        portNo = "80";
        filepath = urlName.substr(slash);
    }

        // if input was given without the closing slash at the end
        // e.g. www.washington.edu
    else {
        serverName = urlName.substr(0, urlName.back());
        portNo = "80";
        filepath = "/";
    }
}

int sendmessage(int socket, char *buffer, int length) {
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

int connection(string portno, string hostname) {
    int sockfd;
    struct addrinfo *results;

    if (getaddrinfo(hostname.c_str(), portno.c_str(), NULL, &results) < 0) {
        cerr << "ERROR unable to resolve host" << endl;
        return 1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "ERROR opening socket" << endl;
        return 1;
    }

    if (connect(sockfd, results->ai_addr, results->ai_addrlen) < 0) {
        cerr << "ERROR connecting" << endl;
        return 1;
    }

    return sockfd;
}

void requestSvr(int sockfd, string serverName, string filePath) {
    stringstream ss;
    string delimiter = "\r\n\r\n";

    ss << "GET " << filePath << " "
       << "HTTP/1.1\r\n"
       << "Host: " << serverName << "\r\n"
       << "Connection: close\r\n"
       << delimiter;
    string request = ss.str();
    sendmessage(sockfd, (char *) request.c_str(), request.size());
    cerr << request;
}

void recvAll(int socket) {
    string header, body = "";
    string delimiter = "\r\n\r\n";
    int contentSize;

    char response[2048];
    memset(response, 0, sizeof(response));
    int n = recv(socket, response, sizeof(response), 0);
    while (n > 0 && (int) body.length() < contentSize) {
        int i = 0;
        while (i < n) {
            if (header.find(delimiter) != string::npos) {
                string::size_type headerSizeStart =
                        header.find("Content-Length: ") + 16;
                string::size_type headerSizeEnd = header.find("\r\n",
                                                              headerSizeStart);
                string contentLength = "";
                contentLength = header.substr(headerSizeStart,
                                              headerSizeEnd - headerSizeStart);
                contentSize = stoi(contentLength);

                body += response[i];
            } else {
                header += response[i];
            }
            i++;
        }
        n = recv(socket, response, sizeof(response), 0);
    }

    cerr << header;
    cout << body;
}
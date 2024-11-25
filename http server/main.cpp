//
//  main.cpp
//  http server
//
//  Created by Nathan Thurber on 23/7/24.
//
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <vector>

#include <thread>

class Server;

int main(int argc, char **argv)
{
    
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }
  
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        std::cerr << "setsockopt failed\n";
        return 1;
    }
  
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4221);
  
    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0)
    {
        std::cerr << "Failed to bind to port 4221\n";
        return 1;
    }
  
    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0)
    {
        std::cerr << "listen failed\n";
        return 1;
    }
    
    int thread_count = 0;
    while (thread_count < 10)
    {
        std::thread thread([&]{
            struct sockaddr_in client_addr;
            int client_addr_len = sizeof(client_addr);
            
            std::cout << "Waiting for a client to connect...\n";
            
            
            int client = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
            
            char msg[65536] = {};
            if (recvfrom(client, msg, sizeof(msg) - 1, 0, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len) == SO_ERROR)
            {
                std::cerr << "listen failed\n";
                return 1;
            }
            
            std::cout << "Client connected\n";
            
            
            std::cout << msg << std::endl;
            
            std::string msg_string(msg);
            
            std::vector<std::string> request;
            std::string buf = {};
            
            int index = 5;
            
            while (index < msg_string.length())
            {
                if (msg_string.at(index) == '/')
                {
                    request.push_back(buf);
                    buf.clear();
                }
                else if (msg_string.at(index) == ' ')
                {
                    request.push_back(buf);
                    buf.clear();
                    break;
                }
                else
                {
                    buf.push_back(msg_string.at(index));
                }
                
                index++;
            }
            
            index = 20;
            std::vector<std::string> headers;
            
            while (index < msg_string.length() - 1)
            {
                if (msg_string.at(index) == ' ')
                {
                    headers.push_back(buf);
                    buf.clear();
                }
                else if (msg_string.at(index) == '\r' && msg_string.at(index + 1) == '\n')
                {
                    headers.push_back(buf);
                    buf.clear();
                    index++;
                }
                else
                {
                    buf.push_back(msg_string.at(index));
                }
                
                index++;
            }
            
            std::string message = std::string("HTTP/1.1 ");
            
            index = 0;
            
            if (request[0] == "echo")
            {
                message += "200 OK\r\n";
                message += "Content-Type: text/plain\r\n";
                message += "Content-Length: ";
                message += std::to_string(request[1].size());
                message += "\r\n\r\n";
                message += request[1];
            }
            else if (request[0] == "user-agent")
            {
                while (index < headers.size() - 1)
                {
                    if (headers[index] == "User-Agent:")
                    {
                        message += "200 OK\r\n";
                        message += "Content-Type: text/plain\r\n";
                        message += "Content-Length: ";
                        message += std::to_string(headers[index + 1].size());
                        message += "\r\n\r\n";
                        message += headers[index + 1];
                        index++;
                    }
                    index++;
                }
            }
            else if (request[0] == "")
            {
                message += "200 OK\r\n";
            }
            else
            {
                message += "404 Not Found\r\n";
            }
            
            message += "\r\n";
            
            send(client, message.c_str(), message.length(), 0);
            
            std::cout << message.c_str() << std::endl;
            return 0;
        });
        
        thread.detach();
        
        thread_count++;
    }
  
    close(server_fd);

    return 0;
}

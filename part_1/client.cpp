#include <iostream> 
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <cmath>

#define MAX_TCP_PACKET 1024

int main() {
    //parameters
    int PORT = 8080;
    std::string server_ip = "127.0.0.1";
    int k = 5;
    int p = 3;

    //map to store word frequencies
    std::map<std::string, int> word_frequencies;

    int client_sockfd = 0;
    struct sockaddr_in server_addr;  
    int addrlen = sizeof(server_addr);

    if ((client_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Socket creation error" << std::endl;
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cout << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    if (connect(client_sockfd, (struct sockaddr *)&server_addr, addrlen) < 0) {
        std::cout << "Connection failed" << std::endl;
        return -1;
    }
    std::cout << "Connection Successful!" << std::endl;

    bool eof_received = false;
    int offset = 0;

    while(!eof_received) {
        std::string message = std::to_string(offset) + "\n";
        if(send(client_sockfd, message.c_str(), message.size(), 0) < 0) {
            std::cout << "Error Sending Message" << '\n';
        } 
        std::cout << "Message sent successfully to transfer " + std::to_string(offset) << '\n';

        std::string buffer_data = "";
        char buffer[MAX_TCP_PACKET];
        ssize_t bytes_received;
        int num_words = 0;

        while(num_words < k) {
            memset(buffer, 0, sizeof(buffer));
            bytes_received = recv(client_sockfd, buffer, sizeof(buffer) - 1, 0);
            if(bytes_received < 0) {
                std::cout << "Error Receiving Message" << '\n';
            }
            buffer[bytes_received] = '\0';
            buffer_data += std::string(buffer);
            // std::cout << buffer_data << '\n';
            size_t newline_pos = buffer_data.find('\n');
            while(newline_pos != std::string::npos) {
                std::string packet = buffer_data.substr(0, newline_pos);
                // std::cout << packet << '\n';
                int pos = 0;
                int start = 0;
                std::string word;
                // Split the packet by commas to get individual words
                while ((pos = packet.find(',', start)) != std::string::npos) {
                    word = packet.substr(start, pos - start);
                    // std::cout << word << '\n';
                    num_words++;
                    // std::cout << num_words;
                    word_frequencies[word]++;
                    start = pos + 1;  
                }
                word = packet.substr(start);
                if (word == "EOF") {
                    eof_received = true; 
                    // std::cout << "Hogya Khatam" << '\n';
                    break;
                } 
                else if (!word.empty()) {
                    num_words++;
                    word_frequencies[word]++;
                }
                buffer_data.erase(0,newline_pos+1);
                newline_pos = buffer_data.find('\n');
            }
            if(eof_received == true) {
                break;
            }
        }
        offset += k;
    }

    close(client_sockfd);
    for(std::map<std::string, int>::const_iterator it = word_frequencies.begin(); it != word_frequencies.end(); ++it)
    {
        std::cout << it->first << ": " << it->second<< "\n";
    }
}
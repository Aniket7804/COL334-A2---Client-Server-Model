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

#define MAX_TCP_PACKET 1024

int main() {
    //parameters
    int PORT = 8080;
    std::string server_ip = "127.0.0.1";
    int k = 5;
    int p = 3;
    int total_words = 0;

    //file pre-processing - creating the memory map
    std::ifstream file("words.txt");
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }
    std::vector<std::string> words;
    std::string line, word;
    while (std::getline(file, line)) {
        // Create a string stream from the line
        std::istringstream ss(line);

        // Extract words separated by commas
        while (std::getline(ss, word, ',')) {
            words.push_back(word);
            total_words++;
        }
    }
    file.close();

    int ori_sockfd, new_sockfd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((ori_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip.c_str(), &address.sin_addr) <= 0) {
        std::cout << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    if (bind(ori_sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(ori_sockfd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server is listening on port " << PORT << std::endl;

    while (true) {
        if ((new_sockfd = accept(ori_sockfd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        std::cout << "Connection Accepted" << '\n';

        int last_packet_sent = false;
        while(!last_packet_sent) {
            char buffer[MAX_TCP_PACKET];
            ssize_t bytes_received = recv(new_sockfd, buffer, MAX_TCP_PACKET - 1, 0);
            if(bytes_received < 0) {
                std::cout << "Error Receiving Message" << '\n';
            }
            buffer[bytes_received] = '\0';
            std::string rec_message = std::string(buffer);
            size_t newline_pos = rec_message.find('\n');
            std::string q_message = rec_message.substr(0, newline_pos);
            int rec_offset = std::stoi(q_message);

            if(rec_offset >= total_words) {
                std::string message = "$$\n";
                break;
            }

            int word_counter = rec_offset;
            while(true) {
                if(word_counter + k < total_words) {
                    int wt = k;
                    while(wt > p) {
                        std::string s_message = "";
                        for(int i = 0; i < p; i++) {
                            s_message = s_message + words[word_counter + i] + ",";
                        }
                        wt -= p;
                        word_counter += p;
                        s_message.pop_back();
                        s_message += "\n";
                        if(send(new_sockfd, s_message.c_str(), s_message.size(), 0) < 0) {
                            std::cout << "Error Sending Message" << '\n';
                        } 
                        std::cout << "packet of p words sent successfully" << '\n';                        
                    }
                    std::string s_message = "";
                    for(int i = 0; i < wt; i++) {
                        s_message = s_message + words[word_counter + i] + ",";
                    }
                    s_message.pop_back();
                    s_message += "\n";
                    if(send(new_sockfd, s_message.c_str(), s_message.size(), 0) < 0) {
                            std::cout << "Error Sending Message" << '\n';
                    } 
                    std::cout << "last packet of (<)p words sent successfully" << '\n'; 
                    break;
                }
                else {
                    int wt = total_words - word_counter;
                    while(wt > p) {
                        std::string s_message = "";
                        for(int i = 0; i < p; i++) {
                            s_message = s_message + words[word_counter + i] + ",";
                        }
                        wt -= p;
                        word_counter += p;
                        s_message.pop_back();
                        s_message += "\n";
                        if(send(new_sockfd, s_message.c_str(), s_message.size(), 0) < 0) {
                            std::cout << "Error Sending Message" << '\n';
                        } 
                        std::cout << "packet of p words sent successfully" << '\n';                        
                    }
                    std::string s_message = "";
                    for(int i = 0; i < wt; i++) {
                        s_message = s_message + words[word_counter + i] + ",";
                    }
                    s_message += "EOF\n";
                    if(send(new_sockfd, s_message.c_str(), s_message.size(), 0) < 0) {
                            std::cout << "Error Sending Message" << '\n';
                    } 
                    last_packet_sent = true;
                    std::cout << "last packet of (<)p words with EOF sent successfully" << '\n';
                    break;
                } 
            }
        }
        close(new_sockfd);
        break;
    }
    close(ori_sockfd);
}


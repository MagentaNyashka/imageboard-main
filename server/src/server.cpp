#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "nlohmann/json.hpp"
#include "../include/DBManager.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>

//using tcp = boost::asio::ip::tcp;

// std::mutex mtx;
// std::condition_variable cv;
// bool stop = false;

std::string JSON_to_string(const std::string& file_name);
inline void startMessage();
int count = 0;


struct ParceServerJson
{
	std::string ipAddress = "0.0.0.0";
	unsigned short port = 8080;

	ParceServerJson()
	{
		try{
			std::string json_data = JSON_to_string("server_data/serveraddress.json"); 
			Json serverJSON = Json::parse(json_data);
			
			ipAddress = serverJSON["ipAddress"].get<std::string>();
			port = (int)serverJSON["port"];
			std::cout << ipAddress << ":" << port << std::endl;
		}
			catch(std::exception const &e){
			std::cerr << "НЕ ПОЛУЧИЛОСЬ СПРАСИТЬ serveraddress.json "<< e.what() << std::endl;
		}
	}	
};


void handleClient(int clientSocket) {
    char buffer[2048];
	memset(buffer, 0, sizeof buffer);
	DBManager* ClientDB = new DBManager();
    try {
		int state = 0;
        while (true && recv(clientSocket, buffer, sizeof(buffer), 0) > 0 && state != -1) {
            std::cout << "Message from client: " << buffer << std::endl;
			state = ClientDB->requestHandler(buffer);
			std::cout << buffer << std::endl;
            send(clientSocket, buffer, strlen(buffer), 0);
			memset(buffer, 0, sizeof buffer);
        }
    } catch (std::exception &e) {
        std::cerr << "Exception occurred while handling client" << std::endl;
    }
	std::cout << "Closing socket..." << std::endl;
	delete ClientDB;
    close(clientSocket);
}


int main() {
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;


	bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));


	listen(serverSocket, 5);

	// int clientSocket = accept(serverSocket, nullptr, nullptr);
	while(true){
		int clientSocket = accept(serverSocket, nullptr, nullptr);
		if (clientSocket != -1) {
			count++;
            std::thread t(handleClient, clientSocket);
            t.detach();
        }
	}
	// close(serverSocket);












	/*
    try {
		startMessage();
		ParceServerJson parcedServerJson; 

        auto const address = boost::asio::ip::make_address(parcedServerJson.ipAddress);
        auto const port = parcedServerJson.port;

        boost::asio::io_context ioc{1};

        tcp::acceptor acceptor{ioc, {address, port}};
        
        while(true){
			tcp::socket socket{ioc};
			acceptor.accept(socket);
			std::cout << "socket connected" << std::endl;
			try{
			std::thread{
				[socket{std::move(socket)}]() mutable{
					boost::beast::websocket::stream<tcp::socket> ws {std::move(const_cast<tcp::socket&>(socket))};
					ws.accept();
					DBManager* ClientDB = new DBManager();
					//std::unique_ptr<DBManager> ClientDB = std::make_unique<DBManager>();
					//auto ClientDB = std::move(ClientDB);
					while(true){
						try{
							
							boost::beast::flat_buffer buffer;
							ws.read(buffer);
							auto message = boost::beast::buffers_to_string(buffer.cdata());
							//std::cout << message << std::endl;
							std::string answer = ClientDB->Request(message);
							buffer.clear();
							boost::beast::ostream(buffer) << answer;
							ws.write(buffer.data());
						}
						catch(boost::beast::system_error const& se){
							if(se.code() != boost::beast::websocket::error::closed){
								std::cout << se.code().message() << std::endl;
								break;
							}
						}
					}
					delete ClientDB;
				} 
          	}.detach();
			std::cout << "closing socket" << std::endl;
			}
			catch(std::exception& e){}
        }

    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
	*/
    return EXIT_SUCCESS;
}



std::string JSON_to_string(const std::string& file_name){
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    std::stringstream buffer;
        buffer << file.rdbuf();
        std::string jsonData = buffer.str();

    return jsonData;
}

/*
inline void startMessage()
{
	std::cout << "TCP SERVER BY CYBERCONNECT(c)  VERSION: "<< VERSION_MAJOR << '.' << VERSION_MINOR << '.' << COMMIT_COUT << std::endl;
}
*/


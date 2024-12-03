#include "DBManager.hpp"


void DBManager::connect(){
    try{
        std::string json_data = JSON_to_string("server_data/serverinfo.json"); 
        Json serverJSON = Json::parse(json_data);
        std::string connection_info = "host="+serverJSON["ipAddress"].get<std::string>()+" port="+std::to_string((int)serverJSON["port"])+" user="+serverJSON["username"].get<std::string>()+" password="+serverJSON["password"].get<std::string>()+" dbname="+serverJSON["database"].get<std::string>();
        c = new pqxx::connection(connection_info);
        std::cout << "[SUCCESS] DBManager CONNECTED: " << c << std::endl;
    }
        catch(std::exception const &e){
        std::cerr << e.what() << std::endl;
    }
}


DBManager::DBManager()
{
    connect();
}

DBManager::~DBManager(){
    std::cout << "Disconnecting..."  "DBManager_ID:" << c << std::endl;
    
    if(c != nullptr){ 
        c->close();
        c = nullptr;
    }
}

//JSON Example: 
/*
{
    "operation":0,
    "username":"hohol1337",
    "password":"da"
}

c

{"operation":3,"id":0}

*/


int DBManager::requestHandler(char* message){
    std::cout << "Handler started" << std::endl;
    Json request;
    try{request = Json::parse(message);}catch(std::exception& e){std::cout << "Error parsing JSON" << std::endl; return 0;}
    int operation;
    try{operation = request["operation"].get<int>();}catch(std::exception& e){std::cout << "Error parsing operation" << std::endl; return 0;}
    int state = 0;
    switch (operation)
    {
    case 0://REG
        state = registerRequest(request);
        break;
    case 1: //LOG
        state = loginRequest(request);
        break;
    case 2: //SEND
        if(username != ""){
            state = sendRequest(request);
        }
        else{
            state = 202;
        }
        break;
    case 3: //SYNC
        //if(username != ""){
            state = syncRequest(request, message);
        //}
        //else{
        //    state = 302;
        //}
        break;
    default:
        return -1;
        break;
    }

    //std::cout << state << std::endl;

    std::string output = "";

    switch (state)
    {
    case 000:
            std::cout << "Registration completed" << std::endl;
            output = "Registration completed";
            break;
    case 001:
            std::cout << "Login has been claimed, cancel..." << std::endl;
            output = "Login has been claimed, cancel...";
            break;
    case 100:
            std::cout << "Authorization completed" << std::endl;
            username = request["username"].get<std::string>();
            output = "Authorization completed";
            break;
    case 101:
            std::cout << "Password incorrect" << std::endl;
            output = "Password incorrect";
            break;
    case 102:
            std::cout << "Login incorrect" << std::endl;
            output = "Login incorrect";
            break;
    case 200:
            std::cout << "Message got sent successfully" << std::endl;
            output = "Message got sent successfully";
            break;
    case 201:
            std::cout << "Error while sending a message" << std::endl;
            output = "Error while sending a message";
            break;
    case 202:
            std::cout << "User did not log in" << std::endl;
            output = "User did not log in";
            break;
    case 300:
            std::cout << "Sync completed" << std::endl;
            output = "Sync completed";
            return state;
            break;
    case 301:
            std::cout << "Sync incompleted" << std::endl;
            output = "Sync incompleted";
            break;
    case 302:
            std::cout << "User did not log in" << std::endl;
            output = "User did not log in";
            break;
    default:
        return -1;
        break;
    }

    std::string result = "";
    Json jsonmassive;
    Json jsonmessage;
    jsonmessage["id"] = -1;
    jsonmessage["content"] = output;
    jsonmessage["author"] = "SERVER";
    jsonmassive.push_back(jsonmessage);
    result += jsonmassive.dump();


    strcpy(message, result.data());
    
    // std::string* temp = new std::string();
    // *temp = std::to_string(state);
    // message = temp->data();
    // strcpy(message, state);

    return state;
}

void reverseJsonArray(nlohmann::json& jsonArray) {
    int n = jsonArray.size();
    for(int i = 0; i < n / 2; ++i) {
        std::swap(jsonArray[i], jsonArray[n - i - 1]);
    }
}

int DBManager::registerRequest(Json request){
    std::string username;
    std::string password;
    try{username = request["username"].get<std::string>();}catch(std::exception& e){}
    try{password = request["password"].get<std::string>();}catch(std::exception& e){}
    if(!username.empty() && !password.empty()){
        std::cout << "registerRequest: " << username << " : " << password << std::endl;
        try{
            pqxx::work w(*c);
            pqxx::result check_username = w.exec("SELECT * FROM users WHERE username='"+username+"';");
            if(check_username.size() == 0){
                std::cout << "Starting registration process..." << std::endl;
                w.exec_params("INSERT INTO users(username, password) VALUES ($1, $2);", username, password);
                w.commit();
                return 0;
            }
            else
            {
                //std::cout << "Login has been claimed, cancel..." << std::endl;
                w.commit();
                return 1;
            }
        }
        catch(std::exception const &e)
        {
            std::cerr << "ОШИБКА" <<  e.what() << std::endl;
        }
        return -1;   
    }
    else{
        return -1;
    }
    return 0;
}

int DBManager::loginRequest(Json request)
{
    std::string username;
    std::string password;
    try{username = request["username"].get<std::string>();}catch(std::exception& e){}
    try{password = request["password"].get<std::string>();}catch(std::exception& e){}
    if(!username.empty() && !password.empty()){
    std::cout << "check Login request: " << username << " : " << password << std::endl;
        try{
            pqxx::work w(*c);
            //std::cout << "faf" << std::endl;
            pqxx::result result_query = w.exec("SELECT password FROM users WHERE username = '"+ username +"';");
            if(result_query.size() > 0){
                if(result_query[0][0].c_str() == password) 
            {
                w.commit();
                return 100;
            }
            else
            {
                w.commit();
                return 101;
            }
            }
            else{
                return 102;
            }
        }
        catch(std::exception const &e)
        {
            std::cerr << e.what() << std::endl;
        }
        return 100;
        }
    return -1;
}

int DBManager::sendRequest(Json request){
    std::string content;
    try{content = request["content"].get<std::string>();}catch(std::exception& e){}
    std::cout << content << " : " << username << std::endl;
    try{
		pqxx::work w(*c);
		w.exec_params("INSERT INTO messages(content, author) VALUES($1, $2);", content, username);
		w.commit();
        return 200;
	}
	catch(const std::exception& e){
		std::cerr << "Failed to send message: " << e.what() << std::endl;
        return 201;
    }
    return 200;
}

int DBManager::syncRequest(Json request, char* message){
    int last_id;
    try{last_id = request["id"].get<int>();}catch(std::exception& e){}
    try{
        pqxx::work w(*c);
        pqxx::result chat_messages = w.exec_params("SELECT id, content, author FROM messages WHERE id > $1 ORDER BY id DESC LIMIT 100", last_id);
	    std::string result = "";
        Json jsonMassive;
        for(auto row : chat_messages)
        {
            Json jsonmessage;
            //std::cout << row[0] << row[1] << row[2] << row[3] << std::endl;
            jsonmessage["id"] = atoi(row[0].c_str());
            jsonmessage["content"] = row[1].c_str();
            jsonmessage["author"] = row[2].c_str();
            jsonMassive.push_back(jsonmessage);
        }
        reverseJsonArray(jsonMassive);
        result += jsonMassive.dump();
        strcpy(message, result.data());
        w.commit();
    }
    catch(std::exception& e)
    {
        std::cerr << "something wrong" << e.what() << std::endl;
    }
    return 300;
}
/*
std::string DBManager::Request(std::string request)
{
    parseJson(request);
    std::string answer="empty";
    switch(type)
    {
        case Register:		// 0
            answer = registerRequest();
            break;
        case getMessage:	// 1
            if(checkLogin())
            {
                answer = getMessageRequest();
            }
            break;
        case sendMessage: 	// 2
            if(checkLogin())
            {
                answer = sendMessageRequest();
            }
	    break;
        case chatCreate:   // 3
            if(checkLogin())
            {
                answer = createChat();
            }
            break;
        case listChats:     // 4
            if(checkLogin())
            {
                answer = getChats();
            }
            break;
        case Login:         // 5
            return checkLoginRequest();

            default:
                return "Инвалид-запрос;(";
                break;
    }
    //std::cout<<answer;
    return answer;

}

std::string DBManager::getChats(){
	std::cout << "Starting getting chats by id..." << std::endl;
	try{
		pqxx::work w(*c);
        Json jsonMassive;
        std::string result = "";
		pqxx::result chats = w.exec_params("SELECT chat_id, members, name FROM chats WHERE $1 = ANY(members)", login);
		for(auto row : chats){
			// for(auto r : row){
			// 	std::cout << r << " ";
			// }
            // std::cout << std::endl;

            Json jsonmessage;
            jsonmessage["chat_id"] = std::stoi(row[0].c_str());
            //jsonmessage["members"] = row[1].c_str();
            jsonmessage["name"] = row[2].c_str();
            jsonMassive.push_back(jsonmessage);
        }
        result += jsonMassive.dump();
        w.commit();
        std::cout<<result;
        return result;
	}
	catch(std::exception& e){
		std::cerr << e.what() << std::endl;
	}
	return "null";
}

std::string DBManager::registerRequest()
{

    std::cout << "registerRequest: " << login << " : " << password << std::endl;
    try{
        std::string result = "err";
        pqxx::work w(*c);
        pqxx::result check_login = w.exec("SELECT * FROM users WHERE login='"+login+"';");
        if(check_login.size() == 0){
            std::cout << "Starting registration process..." << std::endl;
            w.exec_params("INSERT INTO users(login, password, email) VALUES ($1, $2, $3);", login, password, email);
            w.commit();
            return "true";
        }
        else
        {
            std::cout << "Login has been claimed, cancel..." << std::endl;
            w.commit();
            return "false";
        }
    }
    catch(std::exception const &e)
    {
        std::cerr << "ОШИБКА" <<  e.what() << std::endl;
    }
    return "BRUH";
}

std::string DBManager::sendMessageRequest()
{
	std::cout << "Starting sending message..." << std::endl;
	try{
		pqxx::work w(*c);
		std::cout << message << " " << login << " " << chat_id << std::endl;
		w.exec_params("INSERT INTO messages(content, author_id, chat_id) VALUES($1, $2, $3);", message, login, chat_id);
		w.commit();
        return "true";
	}
	catch(const std::exception& e){
		std::cerr << "Failed to send message: " << e.what() << std::endl;
        return "err";
    }
    
}

void reverseJsonArray(nlohmann::json& jsonArray) {
    int n = jsonArray.size();
    for(int i = 0; i < n / 2; ++i) {
        std::swap(jsonArray[i], jsonArray[n - i - 1]);
    }
}

std::string DBManager::getMessageRequest()
{   
    std::cout << "Starting getting messages..." << std::endl;
    try{
        pqxx::work w(*c);
        //pqxx::result chat_messages = w.exec_params("SELECT content, chat_id, author_id, message_id FROM messages WHERE chat_id = $1 LIMIT 25 OFFSET $2;", chat_id, offset * 25);
        pqxx::result chat_messages = w.exec_params("SELECT content, chat_id, author_id, message_id FROM messages WHERE chat_id = $1 ORDER BY message_id DESC LIMIT 25 OFFSET $2;", chat_id, offset * 25);
	std::string result = "";
        Json jsonMassive;
        for(auto row : chat_messages)
        {
            Json jsonmessage;
            //std::cout << row[0] << row[1] << row[2] << row[3] << std::endl;
            jsonmessage["content"] = row[0].c_str();
            jsonmessage["chat_id"] = atoi(row[1].c_str());
            jsonmessage["author_id"] = row[2].c_str();
            jsonmessage["message_id"] = atoi(row[3].c_str());
            jsonMassive.push_back(jsonmessage);
        }
	reverseJsonArray(jsonMassive);
        //std::cout << jsonMassive.dump() << std::endl;
        /*Json json_message;
        json_message["content"] = chat_messages[0][0].c_str();
        json_message["chat_id"] = atoi(chat_messages[0][1].c_str());
        json_message["author_id"] = chat_messages[0][2].c_str();
        json_message["message_id"] = atoi(chat_messages[0][3].c_str());
        json_message["offset"] = offset;
        result += jsonMassive.dump();
        //std::cout << "r: " << result << std::endl;
        w.commit();
        return result;
    }
    catch(std::exception& e)
    {
        std::cerr << "something wrong" << e.what() << std::endl;
    }
    return "somethinig wrong";
}

std::string DBManager::createChat()
{
	std::cout << "Creating chat...    chat_id: " << chat_id << " member0: " << members[0] << " name: " << name << std::endl;
	try{
		pqxx::work w(*c);
		std::string members_to_str = "";
		for(auto member : members){
			members_to_str += "'" + member + "', ";
		}
		members_to_str.pop_back();
		members_to_str.pop_back();
		std::cout << members_to_str << std::endl;
		w.exec("INSERT INTO chats(members, name) VALUES(ARRAY[ " + members_to_str + "], '" + name + "');");
		w.commit();
	}catch(std::exception& e)
    {
        std::cerr << "ОШИБКА createchat()" << e.what() << std::endl;
    }
	return "null";
}

std::string DBManager::checkLoginRequest()
{
    std::cout << "check Login request: " << login << " : " << password << std::endl;
    try{
        std::string result = "err";
        std::cout << "C: " << c << std::endl;
        pqxx::work w(*c);
        //std::cout << "faf" << std::endl;
        pqxx::result result_query = w.exec("SELECT password FROM users WHERE login = '"+ login +"';");
        if(result_query.size() > 0){
          if(result_query[0][0].c_str() == password) 
          {
            std::cout << "password correct" << std::endl;
            w.commit();
            return "true";
          }
          else
          {
            std::cout << "password not correct" << std::endl;
            w.commit();
            return "false";
          }
        }
        else{
          return "false";
        }
        
        
    }
    catch(std::exception const &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return "BRUH";
}

void DBManager::printUsers()
{
    std::cout << "Printing all users:" << std::endl;
    try{
        std::string result = "err";
        pqxx::work w(*c);
        pqxx::result result_query = w.exec("SELECT * FROM users");
        for(auto res : result_query){
            for(auto r : res){
                std::cout << r << " ";
            }
            std::cout << std::endl;
        }
        w.commit();
    }
    catch(std::exception const &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void DBManager::parseJson(std::string request)
{
    std::cout << "Starting parsing..." << std::endl;
    Json json = Json::parse(request);
    try{login = json["username"].get<std::string>();}catch(std::exception& e){}
    try{password = json["password"].get<std::string>();}catch(std::exception& e){}
    try{message = json["message"].get<std::string>();}catch(std::exception& e){}
    try{name = json["name"].get<std::string>();}catch(std::exception& e){}
    try{email = json["email"].get<std::string>();}catch(std::exception& e){}
    try{chat_id = (int)json["chatID"];}catch(std::exception& e){}
    try{type = (int)json["type"];}catch(std::exception& e){}
    try{offset = (int)json["offset"];}catch(std::exception& e){}
    try{
        members = {};
        if (json.contains("members") && json["members"].is_array()) {
        const Json& membersArray = json["members"];

        for (const auto& member : membersArray) {
            members.push_back(member.get<std::string>());
        }
        }
    }catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    

    if(false)
    {
        std::cout << "Parsing completed" << std::endl;
        std::cout << "Login: " << login << std::endl;
        std::cout << "Password: " << password << std::endl;
        std::cout << "Message: " << message << std::endl;
        std::cout << "ChatID: " << chat_id << std::endl;
        std::cout << "type: " << type << std::endl;
        std::cout << "offset: " << offset << std::endl;
    }
}

bool DBManager::checkLogin()
{
    if(checkLoginRequest() == "true")
        return true;
    return false;
}
*/
std::string DBManager::JSON_to_string(const std::string& file_name){
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    std::stringstream buffer;
        buffer << file.rdbuf();
        std::string jsonData = buffer.str();

    return jsonData;
}

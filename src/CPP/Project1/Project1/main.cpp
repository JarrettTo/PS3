#include <SFML/Graphics.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <WS2tcpip.h>

#define WIN32_LEAN_AND_MEAN
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <regex>
#include <set>
#include <unordered_map>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#elif _WIN32_WINNT < 0x0600
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <nlohmann/json.hpp>

#define M_PI 3.14159265358979323846

class Sprite {
private:
    int x;
    int y;
    sf::Texture textures[3];
    int currentImageIndex = 0;
    sf::Font font;
    sf::Text text;

public:
    Sprite(int x, int y) : x(x), y(y) {
        loadImages();

        if (!font.loadFromFile("arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        text.setFont(font);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::Black);
        updateText();
    }
    Sprite() : x(0), y(0) {
        initialize(); 
    }
    

    void initialize() {
      
        loadImages();
        if (!font.loadFromFile("arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        text.setFont(font);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::Black);
        updateText();
    }
    void loadImages() {
        if (!textures[0].loadFromFile("pingu.png") ||
            !textures[1].loadFromFile("pingu2.png") ||
            !textures[2].loadFromFile("pingu.png")) {
            std::cerr << "Failed to load images!" << std::endl;
        }
    }

    void move(int dx, int dy) {
     
        if (x + dx < 0 || x + dx > 1280 || y + dy < 0 || y + dy > 720) {
            sf::RenderWindow messageBox(sf::VideoMode(200, 100), "Warning!");
            sf::Font font;
            sf::Text text;
            if (!font.loadFromFile("arial.ttf")) {
                std::cerr << "Failed to load font!" << std::endl;
            }
            text.setFont(font);
            text.setCharacterSize(20);
            text.setFillColor(sf::Color::Black);
            text.setString("You are trying to move outside the window!");
            text.setPosition(10, 10);
            while (messageBox.isOpen()) {
                sf::Event event;
                while (messageBox.pollEvent(event)) {
                    if (event.type == sf::Event::Closed)
                        messageBox.close();
                }
                messageBox.clear(sf::Color::White);
                messageBox.draw(text);
                messageBox.display();
            }
        }
        else {
            x += dx;
            y += dy;
        }

        if (dx != 0 || dy != 0) {
            currentImageIndex = 1 - currentImageIndex;
        }
        else {
            currentImageIndex = 2;
        }
        updateText();
    }

    void updateText() {
        text.setString("x: " + std::to_string(x) + " y: " + std::to_string(y));
        text.setPosition(static_cast<float>(10), static_cast<float>(10));
    }

    int getX() const {
        return x;
    }

    int getY() const {
        return y;
    }

    void draw(sf::RenderWindow& window) {
        window.draw(text);
        sf::Sprite sprite(textures[currentImageIndex]);

        float scaleX = static_cast<float>(window.getSize().x) / textures[currentImageIndex].getSize().x;
        float scaleY = static_cast<float>(window.getSize().y) / textures[currentImageIndex].getSize().y;

        float scale = std::min(scaleX, scaleY);

        sprite.setScale(scale, scale);

        int centeredX = static_cast<int>((window.getSize().x / 2) - (sprite.getGlobalBounds().width / 2));
        int centeredY = static_cast<int>((window.getSize().y / 2) - (sprite.getGlobalBounds().height / 2));

        sprite.setPosition(static_cast<float>(centeredX), static_cast<float>(centeredY));
        window.draw(sprite);
    }

    void drawScaledSquare(sf::RenderWindow& window, int x, int y) const {
        sf::Color color = sf::Color::Red;
 
        int squareSize = 100; // Example fixed size, adjust as needed

        sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
        square.setPosition(x - squareSize / 2, y - squareSize / 2);
        square.setFillColor(color);

        window.draw(square);
    }
    void drawScaledSprite(sf::RenderWindow& window, int scaledX, int scaledY, int drawPanelWidth, int drawPanelHeight) const {

        int squareSize = 175; 

        sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
        square.setFillColor(sf::Color::Red);
        square.setPosition(scaledX - squareSize / 2, scaledY - squareSize / 2);

        window.draw(square);
    }


};

class Particle {
private:
    double x, y;
    double angle, velocity;

public:
    Particle(double x, double y, double angle, double velocity) : x(x), y(y), angle(angle), velocity(velocity) {}

    void move(double deltaTime) {

        double nextX = x + (-0.3 * velocity) * cos(angle) * deltaTime;
        double nextY = y + (-0.3 * velocity) * sin(angle) * deltaTime;


        if (nextY <= 0 || nextY >= 720) {
            angle = -angle; 
            nextY = y; 
        }

        if (nextX <= 0 || nextX >= 1280) {
            angle = M_PI - angle; 
            angle = fmod(angle, 2 * M_PI); 
            nextX = x;
        }

        x = nextX;
        y = nextY;
    }

    double getX() const {
        return x;
    }

    double getY() const {
        return y;
    }

    void draw(int x, int y, sf::RenderWindow& window) {
        sf::CircleShape circle(10);
        circle.setFillColor(sf::Color::Red);
        circle.setPosition(x, y);
        window.draw(circle);
    }

    void drawScaled(sf::RenderWindow& window, int scaledX, int scaledY, int drawPanelWidth, int drawPanelHeight) const {
        const int peripheryWidth = 33;
        const int peripheryHeight = 19;

        double scaleX = static_cast<double>(drawPanelWidth) / peripheryWidth;
        double scaleY = static_cast<double>(drawPanelHeight) / peripheryHeight;

        double scale = std::min(scaleX, scaleY);
        int spriteWidth = 5;
        int spriteHeight = 5;
        int scaledWidth = static_cast<int>(spriteWidth * scale);
        int scaledHeight = static_cast<int>(spriteHeight * scale);

        sf::CircleShape circle;
        circle.setRadius(scaledWidth / 2.0);
        circle.setPosition(scaledX, scaledY);
        circle.setFillColor(sf::Color::Black);
        window.draw(circle);
    }

};
std::vector<Particle> particles;
std::mutex particlesMutex;
std::unordered_map<std::string, Sprite> clientSprites;
std::mutex clientSpritesMutex;
std::atomic<bool> running(true);
std::atomic<bool> keepRunning(true);
boost::uuids::random_generator generator;
boost::uuids::uuid uuid = generator();
std::string clientId = to_string(uuid);
int peripheryWidth = 33;
int peripheryHeight = 19;
std::mutex heartbeatMutex;
std::condition_variable heartbeatCondition;
std::atomic<bool> receivedHeartbeat(false);
class DrawPanel {
private:
    int peripheryWidth = 33;
    int peripheryHeight = 19;
    Sprite* sprite = nullptr;
    std::vector<Particle> particles;
    std::unordered_map<int, Sprite> clientSprites;
    bool explorer_bool = false;
    double fps = 0.0;
    std::atomic<int> actualFramesDrawn{ 0 };
    sf::RenderWindow window;

    void paintComponent() {
        //explorer();
    }

    void explorer() {
        if (sprite == nullptr) {
            sprite = new Sprite(100, 100);
            return;
        }

        int canvasWidth = 1280;
        int canvasHeight = 720;
        int spriteCenterX = sprite->getX();
        int spriteCenterY = sprite->getY();

        int leftBound = spriteCenterX - peripheryWidth / 2;
        int rightBound = spriteCenterX + peripheryWidth / 2;
        int topBound = spriteCenterY + peripheryHeight / 2;
        int bottomBound = spriteCenterY - peripheryHeight / 2;

        double scaleX = static_cast<double>(canvasWidth) / peripheryWidth;
        double scaleY = static_cast<double>(canvasHeight) / peripheryHeight;

        //sprite->drawScaledSquare();

        for (const Particle& particle : particles) {
            if (particle.getX() + 5 >= leftBound && particle.getX() - 5 <= rightBound &&
                particle.getY() + 5 >= bottomBound && particle.getY() - 5 <= topBound) {
                int relativeX = static_cast<int>(particle.getX()) - leftBound;
                int relativeY = topBound - static_cast<int>(particle.getY());

                int scaledParticleX = static_cast<int>(relativeX * scaleX);
                int scaledParticleY = static_cast<int>(relativeY * scaleY);
                //particle.drawScaled(window, scaledParticleX, scaledParticleY, 1280, 720);
            }
        }

  

        std::cout << "FPS: " << fps << " X: " << spriteCenterX << " Y: " << spriteCenterY << std::endl;
        actualFramesDrawn.fetch_add(1, std::memory_order_relaxed);
    }

    
};
void drawOtherSprites(sf::RenderWindow& window, int centerX, int centerY, double scaleX, double scaleY) {
    std::lock_guard<std::mutex> guard(clientSpritesMutex);

    for (const auto& pair : clientSprites) {
        const Sprite& sprite = pair.second;
        std::cout << "WYH" << std::endl;
  
        int spriteX = sprite.getX() - (centerX - window.getSize().x / 2);
        int spriteY = sprite.getY() - (centerY - window.getSize().y / 2);

        int scaledX = centerX + static_cast<int>(spriteX * scaleX);
        int scaledY = centerY + static_cast<int>(spriteY * scaleY);


        sprite.drawScaledSquare(window, scaledX, scaledY);
    }
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


void processMessageAndUpdateParticles(const std::string& message) {
    std::vector<Particle> tempParticles;
    auto messages = split(message, '}');

    for (auto& singleMessage : messages) {
        
        if (singleMessage.empty()) continue;

        size_t startPos = singleMessage.find('{');
        if (startPos == std::string::npos) continue;
        singleMessage = singleMessage.substr(startPos + 1);

        double x, y, velocity, angle;
        sscanf_s(singleMessage.c_str(),
            "\"x\":%lf,\"y\":%lf,\"velocity\":%lf,\"angle\":%lf",
            &x, &y, &velocity, &angle);

        tempParticles.emplace_back(x, y, velocity, angle);
    }

    std::lock_guard<std::mutex> guard(particlesMutex);
    particles = std::move(tempParticles);
}
void processHeartbeat(const std::string& message) {

    if (message.find("\"heartbeat\":true") != std::string::npos) {
        std::lock_guard<std::mutex> lock(heartbeatMutex);
        receivedHeartbeat = true;
        heartbeatCondition.notify_one();
    }
}
void monitorHeartbeat() {
    while (running) {
        std::unique_lock<std::mutex> lock(heartbeatMutex);
        if (!heartbeatCondition.wait_for(lock, std::chrono::seconds(20), [] { return receivedHeartbeat.load(); })) {
       
            std::cout << "Heartbeat not received, shutting down." << std::endl;
            running = false; 
            break; 
        }
     
        receivedHeartbeat = false;
    }
}
void sendSpritePosition(int x, int y) {
    std::cout << "CLIENT ID" + clientId << std::endl;
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        std::cerr << "WSAStartup failed\n";
        return;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket\n";
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4445);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    std::string message = "{\"clientId\":\"" + clientId + "\",\"spriteX\":" + std::to_string(x) + ",\"spriteY\":" + std::to_string(y) + "}";
    const char* buffer = message.c_str();
    int messageLength = message.length();

    int sentBytes = sendto(clientSocket, buffer, messageLength, 0, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (sentBytes == SOCKET_ERROR) {
        std::cerr << "Error sending message\n";
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    closesocket(clientSocket);
    WSACleanup();
}

void spriteMovement(Sprite& sprite, sf::Event& event) {
    if (event.key.code == sf::Keyboard::W || event.key.code == sf::Keyboard::Up) {
        sprite.move(0, 1);
     
    }
    else if (event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::Left) {
        sprite.move(-1, 0);
       
    }
    else if (event.key.code == sf::Keyboard::S || event.key.code == sf::Keyboard::Down) {
        sprite.move(0, -1);
        
    }
    else if (event.key.code == sf::Keyboard::D || event.key.code == sf::Keyboard::Right) {
        sprite.move(1, 0);
        
    }
    std::thread([=, &sprite]() {
        sendSpritePosition(sprite.getX(), sprite.getY());
        }).detach();
}

void displayWindow(Sprite& sprite, sf::RenderWindow& window) {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                spriteMovement(sprite, event);
            }
        }

        window.clear(sf::Color::White);
        sprite.draw(window);
        window.display();
    }
}

void startUdpClient() {
    try {
        boost::asio::io_context io_context;

        boost::asio::ip::udp::socket socket(io_context);

        boost::asio::ip::udp::resolver resolver(io_context);
        boost::asio::ip::udp::resolver::results_type endpoints = resolver.resolve("230.0.0.1", "4446");

        std::string message = "x:100,y:100";
        socket.open(boost::asio::ip::udp::v4());
        socket.send_to(boost::asio::buffer(message), *endpoints.begin());

        std::cout << "Sent sprite position to server." << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

void listenForShutdownSignal() {
    std::thread listenerThread([&]() {
        int sockfd;
        struct sockaddr_in addr;
        sockaddr_in multicastAddr;
        char buffer[1024];

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("socket");
            return;
        }

        memset(&multicastAddr, 0, sizeof(multicastAddr));
        multicastAddr.sin_family = AF_INET;
        inet_pton(AF_INET, "230.0.0.1", &multicastAddr.sin_addr);
        multicastAddr.sin_port = htons(4448);

        if (bind(sockfd, (struct sockaddr*)&multicastAddr, sizeof(multicastAddr)) < 0) {
            perror("bind");
            closesocket(sockfd);
            return;
        }

        struct ip_mreq mreq;
        inet_pton(AF_INET, "230.0.0.1", &mreq.imr_multiaddr.s_addr);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(mreq)) < 0) {
            perror("setsockopt");
            closesocket(sockfd);
            return;
        }

        while (true) {
            socklen_t addrlen = sizeof(addr);
            int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &addrlen);
            if (n < 0) {
                perror("recvfrom");
                break;
            }

            std::string message(buffer, n);
            if (message.find("\"shutdown\":true") != std::string::npos) {
                std::exit(0);
            }
        }

        closesocket(sockfd);
        });
    listenerThread.detach();
}

void sendDisconnectMessage() {
    std::cout << "CLIENT ID" + clientId << std::endl;
    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(4445);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr.s_addr);


    std::string message = "{\"clientId\":\"" + clientId + "\",\"disconnect\":true}";
    const char* buffer = message.c_str();
    if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        std::cerr << "Send failed" << std::endl;
    }

    closesocket(sockfd);
}
std::vector<std::string> splitOnCommaOutsideBrackets(const std::string& s) {
    std::vector<std::string> parts;
    std::string temp;
    int bracketCount = 0;

    for (char c : s) {
        if (c == '{') bracketCount++;
        if (c == '}') bracketCount--;
        if (c == ',' && bracketCount == 0) {
            parts.push_back(temp);
            temp = "";
        }
        else {
            temp += c;
        }
    }
    if (!temp.empty()) parts.push_back(temp); 

    return parts;
}

void processMessageAndUpdateClientSprites(const std::string& message) {
    using json = nlohmann::json;
    std::unordered_map<std::string, Sprite> newClientSprites;

    try {
        auto jsonArray = json::parse(message);

        for (const auto& item : jsonArray) {
            std::string clientId = item.at("clientId").get<std::string>();
            int x = item.at("x").get<int>();
            int y = item.at("y").get<int>();

            newClientSprites[clientId] = Sprite(x, y);
        }


        {
            std::lock_guard<std::mutex> lock(clientSpritesMutex);
            clientSprites.swap(newClientSprites);
        }

        {
            std::lock_guard<std::mutex> guard(clientSpritesMutex); 
            for (const auto& pair : clientSprites) {
                std::cout << "Client ID: " << pair.first << ", X: " << pair.second.getX() << ", Y: " << pair.second.getY() << std::endl;
            }
        }
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << '\n';
    }
    catch (const json::type_error& e) {
        std::cerr << "JSON type error: " << e.what() << '\n';
    }
}
    

    


void receiveMulticastData(int port, const std::function<void(const std::string&)>& dataHandler) {
    while (keepRunning) {
        SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sockfd == INVALID_SOCKET) {
            std::cerr << "Failed to create socket" << std::endl;
            return;
        }

        int reuse = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
            std::cerr << "Setting SO_REUSEADDR failed" << std::endl;
            closesocket(sockfd);
            return;
        }

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sockfd, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
            std::cerr << "Failed to bind socket" << std::endl;
            closesocket(sockfd);
            return;
        }

        ip_mreq mreq;
        inet_pton(AF_INET, "230.0.0.1", &mreq.imr_multiaddr.s_addr);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) == SOCKET_ERROR) {
            std::cerr << "Failed to join multicast group" << std::endl;
            closesocket(sockfd);
            return;
        }

        char buffer[1000024];
        while (true) {
            sockaddr_in sender;
            int senderLen = sizeof(sender);
            int bytesReceived = recvfrom(sockfd, buffer, sizeof(buffer), 0, reinterpret_cast<SOCKADDR*>(&sender), &senderLen);
            if (bytesReceived == SOCKET_ERROR) {
                std::cerr << "Failed to receive data" << std::endl;
                break;
            }

            std::string receivedJson(buffer, bytesReceived);
            std::cout << receivedJson << std::endl;
            dataHandler(receivedJson);
        }

        closesocket(sockfd);
    }
}

void updateSpritesFromJson(const std::string& json) {
    std::set<std::string> receivedClientIds;
    std::regex pattern("\\{\"clientId\":\"(.*?)\",\"x\":(\\d+),\"y\":(\\d+)\\}");
    std::smatch matcher;
    std::cout << "DATA:" << json << std::endl;

    std::string::const_iterator searchStart(json.cbegin());
    while (std::regex_search(searchStart, json.cend(), matcher, pattern)) {
        std::string clientId = matcher[1];
        int x = std::stoi(matcher[2]);
        int y = std::stoi(matcher[3]);
        receivedClientIds.insert(clientId);

    }
}

void updateParticles(double deltaTime) {
    std::lock_guard<std::mutex> guard(particlesMutex);
    for (auto& particle : particles) {
        particle.move(deltaTime);
    }
}

void particleUpdaterThread() {
    sf::Clock clock;
    while (running) {
        sf::Time elapsed = clock.restart();
        double deltaTime = elapsed.asSeconds();
        updateParticles(deltaTime);
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Explorer Mode");
    
    Sprite sprite(100, 100);
    std::thread updater(particleUpdaterThread);
    startUdpClient();
    listenForShutdownSignal();
    int peripheryWidth = 33;
    int peripheryHeight = 19;

    sf::Clock clock;
    sf::Clock fpsClock; 
    unsigned int frames = 0;
    sf::Text fpsText;
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
    }
    fpsText.setFont(font);
    fpsText.setCharacterSize(24);
    fpsText.setFillColor(sf::Color::Black);
    fpsText.setPosition(5.f, 5.f);
    
    sf::Vector2u windowSize = window.getSize();
    float padding = 10.f;
    float textWidth = fpsText.getLocalBounds().width;
    std::thread receiver([&]() {
        receiveMulticastData(4446, processMessageAndUpdateParticles);
       
       });
    std::thread receiver2([&]() {
        receiveMulticastData(4447, processMessageAndUpdateClientSprites);
        });
    std::thread receiver3([&]() {
        receiveMulticastData(4449, processHeartbeat);
        });
    std::thread heartbeatMonitor(monitorHeartbeat);
    while (window.isOpen() && running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                running = false;
            }
            else if (event.type == sf::Event::KeyPressed)
                spriteMovement(sprite, event);
        }

        window.clear(sf::Color::White);
   
        
        sf::Time elapsed = clock.restart();
        double deltaTime = elapsed.asSeconds();

       
       
        
        int canvasWidth = 1280;
        int canvasHeight = 720;
        int spriteCenterX = sprite.getX();
        int spriteCenterY = sprite.getY();

        int leftBound = spriteCenterX - peripheryWidth / 2;
        int rightBound = spriteCenterX + peripheryWidth / 2;
        int topBound = spriteCenterY + peripheryHeight / 2;
        int bottomBound = spriteCenterY - peripheryHeight / 2;

        double scaleX = static_cast<double>(canvasWidth) / peripheryWidth;
        double scaleY = static_cast<double>(canvasHeight) / peripheryHeight;

        //sprite->drawScaledSquare();

        for (const auto& particle : particles) {
            
            if (particle.getX() + 5 >= leftBound && particle.getX() - 5 <= rightBound &&
                particle.getY() + 5 >= bottomBound && particle.getY() - 5 <= topBound) {
                int relativeX = static_cast<int>(particle.getX()) - leftBound;
                int relativeY = topBound - static_cast<int>(particle.getY());

                int scaledParticleX = static_cast<int>(relativeX * scaleX);
                int scaledParticleY = static_cast<int>(relativeY * scaleY);
                particle.drawScaled(window, scaledParticleX, scaledParticleY, 1280, 720);
            }
        }
        {
            std::lock_guard<std::mutex> guard(particlesMutex);
         
        }
        for (const auto& pair : clientSprites) {
            const std::string& id = pair.first;
            if (id != clientId) {
                const Sprite& sprite = pair.second;
                if (sprite.getX() + 5 >= leftBound && sprite.getX() - 5 <= rightBound &&
                    sprite.getY() + 5 >= bottomBound && sprite.getY() - 5 <= topBound) {
                    int relativeX = static_cast<int>(sprite.getX()) - leftBound;
                    int relativeY = topBound - static_cast<int>(sprite.getY());

                    int scaledParticleX = static_cast<int>(relativeX * scaleX);
                    int scaledParticleY = static_cast<int>(relativeY * scaleY);
                    sprite.drawScaledSprite(window, scaledParticleX, scaledParticleY, 1280, 720);
                }
            }
            
        }
        frames++;
        if (fpsClock.getElapsedTime().asSeconds() >= 0.5f) { 
            fpsText.setString("FPS: " + std::to_string(frames)); 
            frames = 0;
            fpsClock.restart();
        }
        windowSize = window.getSize(); 
        textWidth = fpsText.getLocalBounds().width; 
        fpsText.setPosition(windowSize.x - textWidth - padding, 5.f);
        sprite.draw(window);
        window.draw(fpsText);
       
        window.display();
        if (!running) {
            window.close();
        }
    }
    sendDisconnectMessage();
    updater.join();
    keepRunning = false;
    if (receiver.joinable()) {
        receiver.join();
    }
    if (receiver2.joinable()) {
        receiver2.join();
    }
    if (receiver3.joinable()) {
        receiver3.join();
    }
    heartbeatMonitor.join();

    
    return 0;
}


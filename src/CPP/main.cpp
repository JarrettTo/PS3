#include <SFML/Graphics.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <cstring>
#include <WS2tcpip.h>
#include <regex>
#include <set>
#include <unordered_map>

class Sprite {
private:
    int x;
    int y;
    int currentImageIndex = 0;
    sf::Texture textures[3];
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

    void loadImages() {
        if (!textures[0].loadFromFile("pingu.png") ||
            !textures[1].loadFromFile("pingu2.png") ||
            !textures[2].loadFromFile("pingu.png")) {
            std::cerr << "Failed to load images!" << std::endl;
        }
    }

    void move(int dx, int dy) {
        //x = std::min(std::max(x + dx, 0), 1280);
        //y = std::min(std::max(y + dy, 0), 720);
        // display message box if sprite wants to move outside the edge
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

    void drawScaledSquare(sf::RenderWindow& window, int drawPanelWidth, int drawPanelHeight) {
        sf::Color color = sf::Color::Red;
        int squareSize = std::min(drawPanelWidth, drawPanelHeight) / 4;

        int x = (drawPanelWidth - squareSize) / 2;
        int y = (drawPanelHeight - squareSize) / 2;

        sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
        square.setPosition(x, y);
        square.setFillColor(color);
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
        x += velocity * cos(angle) * deltaTime;
        y += velocity * sin(angle) * deltaTime;
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

    void drawScaled(sf::RenderWindow& window, int scaledX, int scaledY, int drawPanelWidth, int drawPanelHeight) {
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

class DrawPanel {
private:
    int peripheryWidth = 33;
    int peripheryHeight = 19;
    Sprite* sprite = nullptr;
    std::vector<Particle> particles;
    std::unordered_map<int, Sprite> clientSprites;
    bool explorer = false;
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

        drawOtherSprites(spriteCenterX, spriteCenterY, scaleX, scaleY);

        std::cout << "FPS: " << fps << " X: " << spriteCenterX << " Y: " << spriteCenterY << std::endl;
        actualFramesDrawn.fetch_add(1, std::memory_order_relaxed);
    }

    void drawOtherSprites(int centerX, int centerY, double scaleX, double scaleY) {
        int leftBound = centerX - peripheryWidth / 2;
        int rightBound = centerX + peripheryWidth / 2;
        int topBound = centerY + peripheryHeight / 2;
        int bottomBound = centerY - peripheryHeight / 2;

        for (const auto& pair : clientSprites) {
            const Sprite& s = pair.second;

            if (s.getX() + 5 >= leftBound && s.getX() - 5 <= rightBound &&
                s.getY() + 5 >= bottomBound && s.getY() - 5 <= topBound) {
                int relativeX = static_cast<int>(scaleX * (s.getX() - leftBound));
                int relativeY = static_cast<int>(scaleY * (topBound - s.getY()));
                int scaledSpriteX = static_cast<int>(relativeX * scaleX);
                int scaledSpriteY = static_cast<int>(relativeY * scaleY);

                //s.drawScaledSquareOtherSprites(*this, relativeX, relativeY);
            }
        }
    }
};

void sendSpritePosition(int x, int y) {
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

    std::string clientId = "your_client_id";
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
        multicastAddr.sin_addr.s_addr = inet_addr("230.0.0.1");
        multicastAddr.sin_port = htons(4448);

        if (bind(sockfd, (struct sockaddr*)&multicastAddr, sizeof(multicastAddr)) < 0) {
            perror("bind");
            closesocket(sockfd);
            return;
        }

        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr("230.0.0.1");
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

void sendDisconnectMessage(const std::string& clientId) {
    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(4445`);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::string message = "{\"clientId\":\"" + clientId + "\",\"disconnect\":true}";
    const char* buffer = message.c_str();
    if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        std::cerr << "Send failed" << std::endl;
    }

    closesocket(sockfd);
}

void receiveMulticastData(int port, const std::function<void(const std::string&)>& dataHandler) {
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
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
    mreq.imr_multiaddr.s_addr = inet_addr("230.0.0.1");
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(mreq)) == SOCKET_ERROR) {
        std::cerr << "Failed to join multicast group" << std::endl;
        closesocket(sockfd);
        return;
    }

    char buffer[1024];
    while (true) {
        sockaddr_in sender;
        int senderLen = sizeof(sender);
        int bytesReceived = recvfrom(sockfd, buffer, sizeof(buffer), 0, reinterpret_cast<SOCKADDR*>(&sender), &senderLen);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Failed to receive data" << std::endl;
            break;
        }

        std::string receivedJson(buffer, bytesReceived);
        dataHandler(receivedJson);
    }

    closesocket(sockfd);
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


int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    // ... (your existing code for setting up the Sprite and window)

    std::thread clientThread(startUdpClient);
    std::thread shutdownThread(listenForShutdownSignal);

    // Now call displayWindow in the main thread
    displayWindow(sprite, window);

    // Join threads before exiting
    clientThread.join();
    shutdownThread.join();

    // Clean up Winsock
    WSACleanup();
    return 0;
}


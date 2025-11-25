#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>

// Serveur de relais très simple : accepte jusqu'à 4 clients et rediffuse chaque
// ligne reçue (terminée par '\n') à tous les autres. Il ne fait aucune logique
// de jeu, juste du broadcast.

namespace {

std::vector<std::string> listLocalIPs() {
    std::vector<std::string> ips;
    ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) {
        return ips;
    }

    for (ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || !(ifa->ifa_flags & IFF_UP) || (ifa->ifa_flags & IFF_LOOPBACK)) continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            char host[NI_MAXHOST];
            if (getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST) == 0) {
                ips.emplace_back(host);
            }
        }
    }

    freeifaddrs(ifaddr);
    return ips;
}

int createListeningSocket(uint16_t port) {
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }

    if (listen(sock, 8) < 0) {
        perror("listen");
        close(sock);
        return -1;
    }

    return sock;
}

bool sendAll(int sock, const std::string& msg) {
    const char* data = msg.c_str();
    size_t      len  = msg.size();
    size_t      sent = 0;
    while (sent < len) {
        ssize_t n = ::send(sock, data + sent, len - sent, 0);
        if (n <= 0) {
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    return true;
}

} // namespace

int main(int argc, char** argv) {
    uint16_t port = 4000;
    if (argc >= 2) {
        port = static_cast<uint16_t>(std::stoi(argv[1]));
    }

    int listenSock = createListeningSocket(port);
    if (listenSock < 0) {
        std::cerr << "Impossible de démarrer le serveur." << std::endl;
        return 1;
    }

    std::cout << "Serveur Bomberman en écoute sur le port " << port << std::endl;
    auto ips = listLocalIPs();
    if (!ips.empty()) {
        std::cout << "IP locales:";
        for (const auto& ip : ips) {
            std::cout << " " << ip;
        }
        std::cout << std::endl;
    } else {
        std::cout << "IP locales: non trouvées (interfaces inaccessibles)" << std::endl;
    }

    std::map<int, std::string> buffers;  // buffer par client pour découper les lignes
    std::vector<int> clients;

    bool running = true;
    while (running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listenSock, &readfds);
        int maxfd = listenSock;

        for (int c : clients) {
            FD_SET(c, &readfds);
            if (c > maxfd) maxfd = c;
        }

        timeval tv{};
        tv.tv_sec  = 1;
        tv.tv_usec = 0;

        int activity = select(maxfd + 1, &readfds, nullptr, nullptr, &tv);
        if (activity < 0 && errno != EINTR) {
            perror("select");
            break;
        }

        // Nouvelle connexion
        if (FD_ISSET(listenSock, &readfds)) {
            sockaddr_in clientAddr{};
            socklen_t   len = sizeof(clientAddr);
            int         cs  = accept(listenSock, reinterpret_cast<sockaddr*>(&clientAddr), &len);
            if (cs >= 0) {
                clients.push_back(cs);
                buffers[cs] = "";
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
                std::cout << "Client connecté: " << ip << std::endl;
            }
        }

        // Lecture clients
        for (int i = static_cast<int>(clients.size()) - 1; i >= 0; --i) {
            int c = clients[i];
            if (!FD_ISSET(c, &readfds)) continue;

            char buf[1024];
            ssize_t n = recv(c, buf, sizeof(buf), 0);
            if (n <= 0) {
                close(c);
                clients.erase(clients.begin() + i);
                buffers.erase(c);
                std::cout << "Client déconnecté" << std::endl;
                continue;
            }

            std::string& acc = buffers[c];
            acc.append(buf, static_cast<size_t>(n));

            size_t pos;
            while ((pos = acc.find('\n')) != std::string::npos) {
                std::string line = acc.substr(0, pos + 1);
                acc.erase(0, pos + 1);

                // broadcast à tous les clients (y compris l'émetteur pour garder un ordre unique)
                for (int other : clients) {
                    sendAll(other, line);
                }
            }
        }
    }

    for (int c : clients) close(c);
    close(listenSock);
    return 0;
}

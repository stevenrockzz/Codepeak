#include <iostream>
#include <unordered_map>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;

// Global backup map for all servers
unordered_map<string, string> backup_map = {
    {"www.example.com", "192.168.1.1"},
    {"www.openai.com", "192.168.1.2"},
    {"www.google.com", "192.168.1.3"},
    {"www.wikipedia.org", "192.168.1.4"},
    {"www.youtube.org", "192.168.1.5"},
    {"www.extra.org", "192.168.1.6"}
};

// Simulate delay function with failure and delay probabilities
bool simulate_delay(const string& server_type) {
    double delay = static_cast<double>(rand()) / RAND_MAX;

    if ((server_type == "TLD" && delay * 100 > 5) || 
        (server_type == "Root" && delay * 100 > 10)) {
        return false;
    }
    return true; // Simulate success
}

// Authoritative Servers
class AuthoritativeServer {
public:
    virtual string resolve(const string& domain) = 0;
};

class AuthoritativeServerExample : public AuthoritativeServer {
public:
    string resolve(const string& domain) override {
        if (domain == "www.example.com") {
            return "192.168.1.1";
        }
        throw runtime_error("Domain not found in Authoritative Server for www.example.com");
    }
};

class AuthoritativeServerOpenAI : public AuthoritativeServer {
public:
    string resolve(const string& domain) override {
        if (domain == "www.openai.com") {
            return "192.168.1.2";
        }
        throw runtime_error("Domain not found in Authoritative Server for www.openai.com");
    }
};

class AuthoritativeServerGoogle : public AuthoritativeServer {
public:
    string resolve(const string& domain) override {
        if (domain == "www.google.com") {
            return "192.168.1.3";
        }
        throw runtime_error("Domain not found in Authoritative Server for www.google.com");
    }
};

class AuthoritativeServerWikipedia : public AuthoritativeServer {
public:
    string resolve(const string& domain) override {
        if (domain == "www.wikipedia.org") {
            return "192.168.1.4";
        }
        throw runtime_error("Domain not found in Authoritative Server for www.wikipedia.org");
    }
};

class AuthoritativeServerCambridge : public AuthoritativeServer {
public:
    string resolve(const string& domain) override {
        if (domain == "www.youtube.org") {
            return "192.168.1.5";
        }
        throw runtime_error("Domain not found in Authoritative Server for www.youtube.org");
    }
};

// TLD Servers
class TLDServer {
public:
    virtual string resolve(const string& domain) = 0;
};



class TLDServerCom : public TLDServer {
public:
    string resolve(const string& domain) override {
        if (!simulate_delay("TLD")) {
            return "Resolution failed";
        }
        
        cout << "[TLD Server .com] Directing to the appropriate Authoritative Server for domain: " << domain << endl;
        
        if (domain == "www.example.com") {
            return authoritative_server_example.resolve(domain);
        } else if (domain == "www.openai.com") {
            return authoritative_server_openai.resolve(domain);
        } else if (domain == "www.google.com") {
            return authoritative_server_google.resolve(domain);
        } else {
            throw runtime_error("Domain not found in TLD Server .com");
        }
    }

private:
    AuthoritativeServerExample authoritative_server_example;
    AuthoritativeServerOpenAI authoritative_server_openai;
    AuthoritativeServerGoogle authoritative_server_google;
};

class TLDServerOrg : public TLDServer {
public:
    string resolve(const string& domain) override {
        if (!simulate_delay("TLD")) {
            return "Resolution failed";
        }
        
        cout << "[TLD Server .org] Directing to the appropriate Authoritative Server for domain: " << domain << endl;
        
        if (domain == "www.wikipedia.org") {
            return authoritative_server_wikipedia.resolve(domain);
        } else if (domain == "www.youtube.org") {
            return authoritative_server_cambridge.resolve(domain);
        } else {
            throw runtime_error("Domain not found in TLD Server .org");
        }
    }

private:
    AuthoritativeServerWikipedia authoritative_server_wikipedia;
    AuthoritativeServerCambridge authoritative_server_cambridge;
};

// Root Server
class RootServer {
public:
    string resolve(const string& domain) {
        if (!simulate_delay("Root")) {
            return "Resolution failed";
        }
        
        cout << "[Root Server] Directing to the appropriate TLD Server for domain: " << domain << endl;
        
        if (domain.find(".com") != string::npos) {
            return tld_server_com.resolve(domain);
        } else if (domain.find(".org") != string::npos) {
            return tld_server_org.resolve(domain);
        } else {
            throw runtime_error("Unsupported domain extension");
        }
    }

private:
    TLDServerCom tld_server_com;
    TLDServerOrg tld_server_org;
};

// Local DNS Server
class LocalDNSServer {
public:
    LocalDNSServer(int capacity) : capacity(capacity) {}

    string resolve(const string& domain) {
        if (cache.find(domain) != cache.end()) {
            cout << "[Local DNS Server] Cache hit for domain: " << domain << endl;
            return cache[domain];
        }

        cout << "[Local DNS Server] Cache miss for domain: " << domain << endl;
        string ip = "";

        try {
            ip = root_server.resolve(domain);

            if (ip == "Resolution failed") {
                throw runtime_error("Resolution failed at Root Server");
            }

            if (cache.size() >= capacity) {
                cache.erase(cache.begin()); // Simple FIFO cache eviction
            }
            cache[domain] = ip;
            return ip;
        } catch (const runtime_error& e) {
            string error_message = e.what();
            cout << "[Local DNS Server] " << error_message;
                cout << " switching to backup" << endl;
                return backup_map[domain];
        }
    }

    void display_cache() const {
        cout << "[Local DNS Server] Current cache:" << endl;
        for (const auto& entry : cache) {
            cout << entry.first << " -> " << entry.second << endl;
        }
    }

private:
    unordered_map<string, string> cache;
    int capacity;
    RootServer root_server;
};

// Local Device
class LocalDevice {
public:
    LocalDevice(int capacity, LocalDNSServer& dns_server) 
        : capacity(capacity), dns_server(dns_server) {}

    string resolve(const string& domain) {
        if (cache.find(domain) != cache.end()) {
            cout << "[Local Device] Cache hit for domain: " << domain << endl;
            return cache[domain];
        }

        cout << "[Local Device] Cache miss for domain: " << domain << endl;
        string ip = dns_server.resolve(domain);

        if (cache.size() >= capacity) {
            cache.erase(cache.begin()); // Simple cache eviction
        }
        cache[domain] = ip;
        return ip;
    }

    void display_cache() const {
        cout << "[Local Device] Current cache:" << endl;
        for (const auto& entry : cache) {
            cout << entry.first << " -> " << entry.second << endl;
        }
    }

private:
    unordered_map<string, string> cache;
    int capacity;
    LocalDNSServer& dns_server;
};

// Main function
int main() {
    srand(time(nullptr)); // Seed random number generator

    LocalDNSServer local_dns(3); // Local DNS Server cache capacity of 3 entries
    LocalDevice local_device(2, local_dns); // Local Device cache capacity of 2 entries

    // Simulate DNS resolution for multiple domains
    string domains[] = {"www.example.com", "www.google.com", "www.wikipedia.org", "www.openai.com", "www.example.com", "www.extra.org"};

    for (const auto& domain : domains) {
        cout << "Resolving domain: " << domain << endl;
        string ip = local_device.resolve(domain);
        cout << "Resolved IP: " << ip << endl;
        local_device.display_cache();
        cout << endl;
        local_dns.display_cache();
        cout << endl;
    }

    return 0;
}

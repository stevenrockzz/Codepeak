#include <bits/stdc++.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>

using namespace std;

class Node
{
public:
    string key;
    string val;
    Node *prev;
    Node *next;

    Node(string key, string val)
    {
        this->key = key;
        this->val = val;
    }
};

class LRUCache
{
public:
    Node *head = new Node("", "");
    Node *tail = new Node("", "");

    int cap;
    unordered_map<string, Node *> m;

    LRUCache(int capacity)
    {
        cap = capacity;
        head->next = tail;
        tail->prev = head;
    }

    void addNode(Node *newnode)
    {
        Node *temp = head->next;

        newnode->next = temp;
        newnode->prev = head;

        head->next = newnode;
        temp->prev = newnode;
    }

    Node* getHead()
    {
        return head;
    }
    Node* getTail()
    {
        return tail;
    }
    void deleteNode(Node *delnode)
    {
        Node *prevv = delnode->prev;
        Node *nextt = delnode->next;

        prevv->next = nextt;
        nextt->prev = prevv;
    }

    string get(string key)
    {
        if (m.find(key) != m.end())
        {
            Node *resNode = m[key];
            string ans = resNode->val;

            m.erase(key);
            deleteNode(resNode);
            addNode(resNode);

            m[key] = head->next;
            return ans;
        }
        return "";
    }

    void put(string key, string value)
    {
        if (m.find(key) != m.end())
        {
            Node *curr = m[key];
            m.erase(key);
            deleteNode(curr);
        }

        if (m.size() == cap)
        {
            m.erase(tail->prev->key);
            deleteNode(tail->prev);
        }

        addNode(new Node(key, value));
        m[key] = head->next;
    }
};

LRUCache lruCache(5);

string socket_query(string url)
{
    int s, error;
    struct sockaddr_in addr;
    struct hostent *server;

    char const* url1=url.c_str();
    // Create the socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        close(s);
        return "Error 01: creating socket failed!\n";
    }

    // Perform DNS lookup to get the IP address of google.com
    server = gethostbyname(url1);
    if (server == nullptr)
    {
        close(s);
        return "Error 02: no such host found!\n";
    }

    // Set up the server address structure
    bzero((char *)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80); // HTTP port

    bcopy((char *)server->h_addr, (char *)&addr.sin_addr.s_addr, server->h_length);

    // Connect to the server
    error = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if (error != 0)
    {
        close(s);
        return "Error 03: connecting to server failed!\n";
    }

    // Formulate the GET request for google.com
    const char *msg = "GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n";
    // char answ[4096]; // Buffer for response

    // Send the GET request
    send(s, msg, strlen(msg), 0);
    string response;   // This will store the full response
    char buffer[1024]; // Temporary buffer to receive data
    // Receive and print the response
    int received;
    while ((received = recv(s, buffer, sizeof(buffer), 0)) > 0)
    {
        response.append(buffer, received); // Append received data to the string
        if(response.length()>10000)
        {
            break;
        }
    }

    cout << response << endl;
    if (received < 0)
    {
        return "receiving data failed!";
    }

    return response;

    // Close the socket
    close(s);
}

void query_url(string url)
{
    string response = lruCache.get(url);
    if (response != "")
    {
        cout<< response;
    }
    else
    {
        string res=socket_query(url);
        if(res!="receiving data failed!")
        {
            cout<<res;
            lruCache.put(url,res);
        }
        else
        {
            cout<<res;
        }
    }
}

void iterate_LRU()
{
    Node* head=lruCache.getHead();
    Node* tail=lruCache.getTail();

    Node* curr=head->next;
    while(curr!=tail)
    {
        cout<<curr->key;
        cout<<'\n';
        cout<<curr->val;
        curr=curr->next;
    }
}
int main()
{
    query_url("google.com");
    iterate_LRU();
    return 0;
}

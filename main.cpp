#include <enet/enet.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
using namespace std;

ENetAddress address;
ENetHost* server = nullptr;
ENetHost* client = nullptr;

bool CreateServer()
{
    address.host = ENET_HOST_ANY;
    address.port = 1234;
    server = enet_host_create(&address, 32, 3, 0, 0);
    return server != nullptr;
}
bool CreateClient()
{
    client = enet_host_create(NULL, 3, 3, 0, 0);
    return client != nullptr;
}

bool messageReady = false;
char MessageName[999];
bool winnerExists = false;

void MessageIntake()
{
    cin.ignore(999, '\n');
    while (1)
    {
        cin.getline (MessageName, 999, '\n');
        messageReady = true;
        this_thread::sleep_for(chrono::milliseconds(500));
    }  
   
}

int main(int argc, char** argv)
{
    // Initial Setup
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        cout << "An error occurred while initializing ENet." << endl;
        return EXIT_FAILURE;
    }

    cout << "1) Create Server " << endl;
    cout << "2) Create Client " << endl << endl;
    int UserInput;
    cin >> UserInput;

    cout << "Welcome to the number guessing game!" << endl;
    cout << "Try to guess the secret number between 0 to 9." << endl;
    cout << "Type your chat messages below!" << endl;
    cout << "Type QUIT to quit." << endl << endl;
    
    thread FirstThread(MessageIntake);
    FirstThread.detach();

    ENetAddress address;
    ENetEvent event;
    ENetPacket* packet;


    // Connections Setup
    if (UserInput == 1)
    {
        if (!CreateServer())
        {
            fprintf(stderr,
                "An error occurred while trying to create an ENet server host.\n");
            exit(EXIT_FAILURE);
        }
    }
    if (UserInput == 2)
    {
        if (!CreateClient())
        {
            fprintf(stderr,
                "An error occurred while trying to create an ENet client host.\n");
            exit(EXIT_FAILURE);
        }

        ENetPeer* peer;

        enet_address_set_host(&address, "127.0.0.1");
        address.port = 1234;
        peer = enet_host_connect(client, &address, 2, 0);

        if (peer == NULL)
        {
            fprintf(stderr, "No available peers for initiating an ENet connection.\n");
            exit(EXIT_FAILURE);
        }

        if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
        {
            cout << "Connection to 127.0.0.1:1234 succeeded." << endl;
        }

        else
        {
            enet_peer_reset(peer);
            cout << "Connection to 127.0.0.1:1234 failed." << endl;
        }
    }


    // Host Code
    if (UserInput == 1)
    {
        srand(time(0));
        int randomNumber = rand() % 9 + 1;
        char secretNumber = '0' + randomNumber;

        cout << "The secret number is: " << secretNumber << endl;

        string Received;
        char firstCharacter;
        

        while (1)
        {
            if (MessageName[0] == 'Q' && MessageName[1] == 'U' && MessageName[2] == 'I' && MessageName[3] == 'T')
            {
                cout << endl << endl << "Exiting Program" << endl << endl;
                return 0;
            }
            if (messageReady)
            {
                string Message = MessageName;
                packet = enet_packet_create(Message.c_str(), Message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(server, 0, packet);
                enet_host_flush(server);
                messageReady = false;
            }

            while (enet_host_service(server, &event, 500) > 0)
            {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_CONNECT:
                    cout << "A new client connected. " << endl;
                    event.peer->data = (void*)("Client information");
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    cout << (char*)event.peer->data << "disconnected." << endl;
                    event.peer->data = NULL;
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    Received = (char*)event.packet->data;
                    firstCharacter = Received.at(0);
                    cout << "Incoming Guess: " << firstCharacter << endl;
                    enet_packet_destroy(event.packet);

                    if (firstCharacter == secretNumber)
                    {
                        cout << endl << "THE NUMBER WAS GUESSED!" << endl;
                        ENetPacket* packet;

                        string Message = "WE HAVE A WINNER!";
                        packet = enet_packet_create(Message.c_str(), Message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                        // packet = enet_packet_create("WE HAVE A WINNER!", strlen("WE HAVE A WINNER!") + 1, ENET_PACKET_FLAG_RELIABLE);
                        enet_host_broadcast(server, 0, packet);
                        enet_host_flush(server);
                        messageReady = false;
                        return 0;
                    }
                    break;
                }
            }
        }



    }

    // Client Code
    if (UserInput == 2)
    {
        string Received;
        char firstCharacter;
        while (1)
        {
            if (MessageName[0] == 'Q' && MessageName[1] == 'U' && MessageName[2] == 'I' && MessageName[3] == 'T')
            {
                cout << endl << endl << "Exiting Program" << endl << endl;
                return 0;
            }
            if (messageReady)
            {
                string Message = MessageName;
                packet = enet_packet_create(Message.c_str(), Message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(client, 0, packet);
                enet_host_flush(client);
                messageReady = false;
            }


            while (enet_host_service(client, &event, 500) > 0)
            {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_RECEIVE:
                    Received = (char*)event.packet->data;
                    cout << Received << endl;
                    enet_packet_destroy(event.packet);

                    if (Received == "WE HAVE A WINNER!")
                        return 0;
                }
            }

        }
    }


    // Post-Program Clean-up
    if (server != nullptr)
        enet_host_destroy(server);
    if (client != nullptr)
        enet_host_destroy(client);
    return EXIT_SUCCESS;
}
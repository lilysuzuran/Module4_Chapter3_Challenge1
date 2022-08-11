#include <enet/enet.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <conio.h>
#include <mutex>

using namespace std;

ENetAddress address;
ENetHost* server = nullptr;
ENetHost* client = nullptr;

bool messageReady = false;
bool guessProcessing = false;
char MessageName[999];

void MessageIntake()
{
    cin.ignore(999, '\n');
    while (1)
    {
        if (guessProcessing == false)  // TODO: I can use Mutex to .lock() Message Intake instead of global variable, if I get extra time
        {
            cin.getline(MessageName, 999, '\n');
            messageReady = true;
            this_thread::sleep_for(chrono::milliseconds(500));
            guessProcessing = true;
        }
    }
}
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

// TODO: I can clean up most of this code into classes and store them in separate header and cpp files
// However I'm really far behind on my other Level Up U stuff like McKinsey and Game Dev 101 tasks so I just made a working program
// On the job I'm sure this would be tech debt I'd need to clean up when there's time, but Level Up U is super fast paced right now...

int main(int argc, char** argv)
{
    // Initial Setup
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        cout << "An error occurred while initializing ENet." << endl;
        return EXIT_FAILURE;
    }

    cout << endl;
    cout << "Welcome to the Number Guessing Game!" << endl;
    cout << "Please select whether you want to set up a Server, or join one as a Player." << endl;
    cout << endl;
    cout << "1) Create a Server as a non-Player " << endl;
    cout << "2) Join a Server as a Player " << endl; 
    cout << endl;

    int UserInput;
    cin >> UserInput;
    system("cls");
    
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

        cout << "You have started a Number Guessing Game Server." << endl;
        cout << "As the Game Host, any guesses you make will not be considered." << endl;
        cout << endl;
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
            fprintf(stderr, "There were no available Number Guessing Game Servers.\n");
            exit(EXIT_FAILURE);
        }

        if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
        {
            cout << "You have joined a Number Guessing Game Server." << endl;
            cout << endl;
        }

        else
        {
            enet_peer_reset(peer);
            cout << "Connecting to a Number Guessing Game Server failed." << endl;
        }
    }

    cout << "Game Rules:" << endl;
    cout << "There is a secret number between 0 to 9." << endl;
    cout << "Once the game starts, Players can type in their guesses." << endl;
    cout << "The game starts when two Players have joined." << endl;
    cout << endl;
    cout << "Type in your guesses below!" << endl;
    cout << "Type QUIT to quit." << endl;
    cout << endl;


    // Host Code
    if (UserInput == 1)
    {
        srand(time(0));
        int randomNumber = rand() % 9 + 1;
        char secretNumber = '0' + randomNumber;

        cout << "The secret number is: " << secretNumber << endl;

        string Received;
        char firstCharacter = 'x';
        int currentPlayers = 0;
        

        while (1)
        {
            guessProcessing = false;

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
                cout << "Reminder: As the Host, your guesses will not be considered." << endl;
            }

            while (enet_host_service(server, &event, 500) > 0)
            {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_CONNECT:
                    cout << "A new player connected. " << endl;
                    event.peer->data = (void*)("Client information");

                    currentPlayers += 1;

                    ENetPacket* packet;
                    if (currentPlayers == 1)
                    {
                        string Message = "1 out of 2 needed players have connected.";
                        packet = enet_packet_create(Message.c_str(), Message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                        enet_host_broadcast(server, 0, packet);
                        enet_host_flush(server);
                    }
                     
                    if (currentPlayers > 1)
                    {
                        string Message = "We have enough players! Time to start guessing!";
                        packet = enet_packet_create(Message.c_str(), Message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                        enet_host_broadcast(server, 0, packet);
                        enet_host_flush(server);

                        cout << Message << endl;
                    }  

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

                    if (currentPlayers < 2)
                    {
                        ENetPacket* packet;
                        string Message = "The game has not yet started; guesses will not be considered.";
                        packet = enet_packet_create(Message.c_str(), Message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                        enet_host_broadcast(server, 0, packet);
                        enet_host_flush(server);
                    }

                    if (currentPlayers == 2)
                    {
                        ENetPacket* packet;
                        string NumberGuessed(1, firstCharacter);
                        string Message = "A player has guessed the following number: " + NumberGuessed;
                        packet = enet_packet_create(Message.c_str(), Message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                        enet_host_broadcast(server, 0, packet);
                        enet_host_flush(server);
                    }

                    if (currentPlayers == 2 && firstCharacter != secretNumber)
                    {
                        ENetPacket* packet;
                        string Message = "That was not the correct number. ";
                        packet = enet_packet_create(Message.c_str(), Message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                        enet_host_broadcast(server, 0, packet);
                        enet_host_flush(server);
                    }

                    if (currentPlayers == 2 && firstCharacter == secretNumber)
                    {
                        ENetPacket* packet;
                        string Message = "THIS IS THE CORRECT NUMBER! WE HAVE A WINNER!";
                        packet = enet_packet_create(Message.c_str(), Message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                        enet_host_broadcast(server, 0, packet);
                        enet_host_flush(server);

                        cout << endl << endl << "THE NUMBER WAS GUESSED! ENDING THE GAME." << endl;
                        return 0;
                    }

                    {
                        ENetPacket* packet;
                        string Message = "heard";
                        packet = enet_packet_create(Message.c_str(), Message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send(event.peer, 0, packet);
                        enet_host_flush(server);
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
                    enet_packet_destroy(event.packet);

                    if (Received != "heard")
                    {
                        cout << Received << endl;
                    }

                    if (Received == "THIS IS THE CORRECT NUMBER! WE HAVE A WINNER!")
                    {
                        cout << endl << endl << "The Game is Over, Thanks for Playing!!" << endl;
                        break;
                    }
                }
            }

            if (Received == "heard")
            {
                guessProcessing = false;
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
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <queue>
#include <memory>
#include <unistd.h>
#include <packet_structures.h> 

#define SERVER_PORT 3000
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 1024


/*
    Creates a TCP socket connection and returns socket_id
*/
int create_socket_connection()
{
    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent* host = gethostbyname(SERVER_ADDR); 

    sockaddr_in server_addr;
    bzero((char *)&server_addr, sizeof(sockaddr_in));
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;


    int status = connect(socket_id,
                            (sockaddr*) &server_addr, sizeof(server_addr));

    if (status < 0)
    {
        std::cout << "Failed to connect to TCP server. " << strerror(errno) << std::endl;
        std::cout << "\n-------- Exiting --------------\n" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "\n-----------CONECTED-----------------\n" << std::endl;

    return socket_id;
}


/*
    Verifies if the received packet is the packet which we requested for.
    - If it is process it and update the missed_seqs. Returns True.
    - Else print error and req for the same packet again. Returns false.

*/
bool process_missed_data(request_packet* req_packet, response_packet* packet, 
                        std::queue<std::unique_ptr<missed_packet>>& missed_seqs)
{
    bool sucessfully_recovered = false;
    if (packet->seq != req_packet->resentType)
    {
        std::cout << "Received wrong packet during recovery, req_seq: " 
            << (int)req_packet->resentType << " recv_seq:" << packet->seq << std::endl;
    }
    else
    {
        if (missed_seqs.front()->start == packet->seq)
        {
            ++missed_seqs.front()->start;
        }

        // Start >= End indicates that we have recovered this missed packets.
        if (missed_seqs.front()->start >= missed_seqs.front()->end)
        {
            missed_seqs.pop();
        }

        sucessfully_recovered = true;
        packet->verify_data();
    }

    return sucessfully_recovered;
}


/*
    Processes received packet and updates prev_seq.
    If there is any seq missed adds it ti missed_seq queue.
*/
void process_data(int& prev_seq, response_packet* packet, std::queue<std::unique_ptr<missed_packet>>& missed_seqs)
{
    if (packet->seq != prev_seq + 1)
    {
        std::unique_ptr<missed_packet> p = std::make_unique<missed_packet>(prev_seq + 1, packet->seq);
        std::cout << "\nMissed packets from: " << prev_seq + 1 << " to: " << packet->seq << "\n" << std::endl;
        missed_seqs.push(std::move(p));
    }

    packet->verify_data();
    prev_seq = packet->seq;
}

/*
    Update request packet to request for the missed seqs.
*/

void update_request(bool& req_data, bool& connect, request_packet* req_packet, 
                    std::queue<std::unique_ptr<missed_packet>>& missed_seqs)
{
    if (!missed_seqs.empty())
    {
        req_packet->callType = 2;
        req_packet->resentType = missed_seqs.front()->start;
        req_data = true;
        connect = true;
        return;
    }

    connect = false;
    req_data = false;
}

int main()
{
    int socket_id;
    bool req_data = true;
    bool connect = true;
    bool in_backlog_recovery = false;
    int32_t prev_seq = 0;
    char* buffer = new char[BUFFER_SIZE];

    request_packet req_packet;
    req_packet.callType = 1;
    req_packet.resentType = 0;
    
    std::queue<std::unique_ptr<missed_packet>> missed_seqs;

    while (true)
    {
        memset(buffer, '\0', sizeof(BUFFER_SIZE));

        if (connect)
        {
            socket_id = create_socket_connection();
            connect = false;
        }

        if (req_data)
        {
            int n = send(socket_id, (const void* ) &req_packet, sizeof(req_packet), 0);
            if (n < 0)
            {
                std::cout << "Failed to send req to server. " << strerror(errno) << std::endl;
            }
            else
            {
                std::cout << "Request for callType: " << (int) req_packet.callType << 
                            " resentType: " << (int) req_packet.resentType << std::endl;
                req_data = false;
            }
        }

        int data = recv(socket_id, buffer, BUFFER_SIZE, MSG_DONTWAIT);

        if (data > 0) // Received single packet
        {
            response_packet* resp_packet = (response_packet*) buffer;
            std::cout << "Received " << resp_packet << std::endl;

            if (req_packet.callType == 1)
            {
                process_data(prev_seq, resp_packet, missed_seqs);
            }
            else
            {
                process_missed_data(&req_packet, resp_packet, missed_seqs);
                close(socket_id);

                if (in_backlog_recovery && missed_seqs.empty())
                {
                    std::cout << "\nReceived all Missed Packets." << std::endl;
                    break;
                }
                update_request(req_data, connect, &req_packet, missed_seqs);
            }


        }
        else if (data == 0) // Successfully received all the data.
        {        
            close(socket_id);
            if (missed_seqs.empty())
            {
                std::cout << "\nReceived all Packets without any miss." << std::endl;
                break;
            }
            std::cout << "\nRequesting for missed sequence\n" << std::endl;
            update_request(req_data, connect, &req_packet, missed_seqs);
            in_backlog_recovery = true;
        }
        else if (errno != EAGAIN) // We can ignore the EAGAIN error as we have non blocking sockets
        {
            if (in_backlog_recovery && missed_seqs.empty())
            {
                std::cout << "\nReceived all Missed Packets." << std::endl;
                break;
            }

            std::cout << strerror(errno) << std::endl;
            close(socket_id);

            std::cout << "\nError while receiving data. Restarting....\n" << std::endl;
            
            while (!missed_seqs.empty())
            {
                missed_seqs.pop();
            }

            req_packet.callType = 1;
            req_packet.resentType = 0;
            req_data = true;
            connect = true;
        }

    }

    std::cout << "\n--------------ShutDown-------------------" << std::endl;

    delete buffer;
}
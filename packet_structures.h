#ifndef PACKET_STRUCTURE
#define PACKET_STRUCTURE
#include <iostream>

struct request_packet
{
    int8_t callType;
    int8_t resentType;
};

struct missed_packet
{
    int32_t start;
    int32_t end;

    missed_packet(int32_t _start, int32_t _end)
            : start(_start), end(_end)
    {}
};

struct response_packet
{
    char symbol[4];
    char buy_sell_indicator;
    int32_t qty;
    int32_t price;
    int32_t seq;

    void verify_data()
    {
        bool is_valid = true;
        std::cout << "Verifying order for Seq_no " << seq << " : ";
        
        if (this->buy_sell_indicator != 'S' && this->buy_sell_indicator != 'B')
        {
            std::cout << " Invalid Side: " << this->buy_sell_indicator;
            is_valid = false;
        }

        if (this->price <= 0)
        {
            std::cout << " Invalid Price: " << this->price;
            is_valid = false;
        }

        if (this->qty % 5 != 0 || this->qty > 100000) // here 100000 is only for example actually it will be based on contract.
        {
            std::cout << " Invalid Qty: " << this->qty; 
            is_valid = false;
        }

        if (is_valid)
        {
            std::cout << " Valid Order.";
        }

        std::cout << std::endl;
       
    }

    friend std::ostream& operator<<(std::ostream&, const response_packet * packet)
    {
        std::cout << "Seq: " << packet->seq << " Symbol: " << packet->symbol
                << " Price: " << packet->price << " Quantity: " << packet->qty
                << " buy_sell_indicator: " << packet->buy_sell_indicator;

        return std::cout;
    }
};


#endif
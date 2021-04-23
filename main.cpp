#include <iostream>
#include <unordered_map>
#include <fstream>
#include <thread>

//boost includes
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>

using boost::uuids::detail::md5;

static const size_t COUNT_NUMBERS = 100000000;
//6237245 - примерно такое число даёт сгенерировать система

//#define SERIALIZE

void serialise_to_file(const char* path, std::unordered_map<std::string, std::string>& hash_and_pins);

std::string from_digits_to_string(const md5::digest_type& digest)
{
    const auto char_digest = reinterpret_cast<const char*>(&digest);
    std::string result;
    boost::algorithm::hex(char_digest, char_digest + sizeof(md5::digest_type), std::back_inserter(result));
    return result;
}

void generator_to_map()
{
    std::unordered_map<std::string, std::string> hash_pins;
    std::cout << "Start of hashing 1\n";
    size_t generator_number = COUNT_NUMBERS;
    for (size_t i = 0; i < COUNT_NUMBERS / 20; ++i)
    {
        std::string pin = std::to_string(++generator_number);
        pin.erase(pin.begin(), pin.begin() + 1);
        md5 hash;
        md5::digest_type digest;
        hash.process_bytes(pin.data(), 8);
        hash.get_digest(digest);
        std::string my_hash = from_digits_to_string(digest);
        hash_pins.insert({ my_hash, pin });
        std::cout << i << std::endl;
    }
    std::cout << "End of hashing 1\n";

    serialise_to_file("serialised_pins.txt", hash_pins);
}

void serialise_to_file(const char* path, std::unordered_map<std::string, std::string>& hash_and_pins)
{
    std::ofstream out;
    out.open(path, std::ios::out);
    if (out.is_open())
    {
        std::cout << "serialising to file\n";
        boost::archive::text_oarchive oarch(out);
        oarch << hash_and_pins;
        out.close();
    }
    else
    {
        std::cout << "Could not open file\n";
    }
    std::cout << "End of serializing\n";
}

void deserialize_from_file(const char* path, std::unordered_map<std::string, std::string>& deserialized_hash_pins)
{
    std::ifstream input_file;
    input_file.open(path, std::ios::in);
    if (input_file.is_open())
    {
        std::cout << "\nstart of deserializing\n";
        boost::archive::text_iarchive iarch(input_file);
        iarch >> deserialized_hash_pins;
        input_file.close();
        std::cout << "\nend of deserializing\n";
    }
    else
    {
        std::cout << "Could not open file\n";
    }
}

int main() 
{
#ifdef SERIALIZE
    std::unordered_map<std::string, std::string> hash_pins;
    generator_to_map();

    exit(0);
#endif //SERIALIZE
#ifndef SERIALIZE
    std::unordered_map<std::string, std::string> deserialized_hash_pins;
    std::thread th_deserialize([&]() {deserialize_from_file("serialised_pins.txt", deserialized_hash_pins); });
    bool is_load_map = false;
    std::thread th_preload([&]()
        {
            while (!is_load_map)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << '*';
            }
            std::cout << "\n\n";
        });

    th_deserialize.join();
    is_load_map = true;
    th_preload.detach();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::string user_pin;
    while (user_pin != "#") 
    {
        std::cout << "Input: ";
        std::getline(std::cin, user_pin);
        md5 hash;
        md5::digest_type digest;
        hash.process_bytes(user_pin.data(), user_pin.size());
        hash.get_digest(digest);
        std::string user_hash = from_digits_to_string(digest);
        std::cout << "Your hash: " << user_hash << "\tYour PIN: " << deserialized_hash_pins[user_hash] << std::endl;
    }
#endif //SEIRIALIZE
    return 0;
}
#ifndef BASE64_H
#define BASE64_H
#include <vector>
#include <string>
using std::string;
using std::vector;

//our namespace, all functions and variables inside it
//have Base64 namespace's scope
//and we refer to them via Base64::SomeFunction or Base64::Variable
namespace Base64
{
//prototypes for functions
std::string base64_encode(const std::string &);
std::string base64_decode(const std::string &);

//these are arbitrary "salts" which will be used
//to obfuscate the original string
const std::string &SALT1 = "LM::TB::BB";
const std::string &SALT2 = "__:/__77";
const std::string &SALT3 = "line=wowC++";

std::string EncryptB64 (std::string s)
    {
        //this is arbitrary, we can mix it... but, we have to use the SAME salts
        //and the same steps when decoding (1)
        s = SALT1 + s + SALT2 + SALT3;
        //encode it, 1st time
        s = base64_encode (s); //this function is defined below
        //add more salt
        //this time, we will inset salt after 7th character (2.1)
        s.insert (7, SALT3);
        //add salt1 on the end (2.2)
        s += SALT1;
        //encode it, 2nd time
        s = base64_encode (s);
        //and.. more salt, this all is arbitrary (3)
        s = SALT2 + SALT3 + s + SALT1;
        //and, encode it one more (and last) time
        s = base64_encode (s);
        //also, crucial step is that we obfuscate this base64 string
        //that no one is able to decode it with a regular base64 decoder
        //and find out that the decoded string is another base64 string
        //with salt... (and, maybe, remove that salt and recover our 'secret' text)
        //so, with adding next characters we are "destroying"
        //a regular base64 string, it is not valid anymore (until added character are removed)
        s.insert (1, "L"); //string::insert return a reference, so it WILL
        s.insert (7, "M"); //modify the original string, no need for assignment
        return s;
    }
//in this function we have to reverse the steps
//taken in the EncryptB64 function
//#parameter: (triple) decoded (and salted) string
std::string DecryptB64 (std::string s)
    {
        //so, first, we need to remove the 'M'
        //this is step (4) from previous function
        //string::erase function erases portion of a string
        //parameters are beginning of the portion and length
        s = s.erase (7, 1); //erase 1 character at position 7
        s = s.erase (1, 1); //erase 1 character at position 1
        //then decode it, 1st time
        s = base64_decode (s);
        //we need to remove SALT2 and SALT3 from beginning (3)
        //string::substr function returns the sub string
        //parameters are beginning and length (optional, default value is to the end)
        s = s.substr (SALT2.length() + SALT3.length());
        //remove SALT1 from the end
        s = s.substr (0, s.length() - SALT1.length());
        //and decode it for 2nd time
        s = base64_decode (s);
        //remove SALT1 from end (step 2.2)
        s = s.substr (0, s.length() - SALT1.length());
        //and SALT3 from position 7 (2.1)
        s = s.erase (7, SALT3.length());
        //and, finally, last decoding, 3rd
        s = base64_decode (s);
        //remove SALT1 from beginning (1)
        s = s.substr (SALT1.length());
        //and SALT2 and SALT3 from end
        s = s.substr (0, s.length() - SALT2.length() - SALT3.length());
        //return decrypted string from function
        return s;
    }

//all chars. we'll map input to these 64 chars
//"A" is at index 0, "B" at index 1 and so on... "/" is at index 63
const std::string &BASE64_CODES = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//function for encoding string into base64 string
//#parameter s --> input string to encode
std::string base64_encode (const std::string &s)
    {
        //the output string. We'll fill it with encoded chars...
        std::string ret;
        //val will be the index we use to map input to our table (BASE64_CODES)
        int val = 0;
        //bits will be used to represent number of bits in a subgroup
        int bits = -6;
        //we will use this to binary AND shifted 'val' to get the right index
        const unsigned int b63 = 0x3F; //0x3F is hexadecimal, decimal value is 63
        //this is a foreach loop (new in C++11)
        //we use it to iterate through string "s" (string to decode)
        //to "c" is assigned every character from "s" string
        for (const auto &c : s)
            {
                val = (val << 8) + c;
                //add 8 to number of bits, base64 works with octets
                //when extracting information from original string (char)
                bits += 8;
                while (bits >= 0)
                    {
                        //in resulting string insert the mapped number
                        //>> means right shifting, & means binary AND
                        //101 & 011 = 001, so only 1 & 1 = 1, other combinations
                        //are zero
                        ret.push_back(BASE64_CODES[(val >> bits) & b63]);
                        bits -= 6; //decrease number of bits by 6
                        //reason for this is grouping by 6 bits
                    }
            }
        if (bits > -6) //at least one char is inserted (initial value for "bits" was -6)
            //insert it
            ret.push_back(BASE64_CODES[((val << 8) >> (bits + 8)) & b63]);
        //if ret size is not aligned (we have padding)
        //just insert "=" or "==" at the end and the decoder will know
        while (ret.size() % 4)
            ret.push_back('=');
        return ret;
    }
//function for decoding string into base64 string
//#parameter s --> input string to decode
std::string base64_decode(const std::string &s)
    {
        //output string
        std::string ret;
        //create a temp vector, assume only characters
        //in range [0, 255] will be in input string
        //size is 256, and all element are -1
        std::vector<int> vec(256, -1);
        //fill it with values from our table
        //notice that index for "vec" is mapped value
        for (int i = 0; i < 64; i++)
            vec [BASE64_CODES[i]] = i;
        //same as in base64_encode
        //this time, bits are 8 (reversing the process)
        //starting with -8, and adding 6 in foreach loop
        //in base64_encode we started with -6 and were adding 8
        int val = 0, bits = -8; //octet - 1byte
        for (const auto &c : s)
            {
                //character not found, exiting for loop
                //this means
                if (vec[c] == -1) break;
                val = (val << 6) + vec[c]; //left shift by 6 bits
                bits += 6; //increase by 6, reversing the step
                           //taken in encoding process
                if (bits >= 0)
                    {
                        //add it to decoded string
                        //we cast it to char because "val" and "bits" are
                        //integer variables (so is char)
                        //(val >> bits) is implicitly casted to char
                        //0xFF is hexadecimal, decimal value is 255
                        //that is the max char value (and vector size)
                        ret.push_back(char((val >> bits) & 0xFF));
                        bits -= 8;
                    }
            }
        //returning the decoded string
        return ret;
    }
}
#endif // BASE64_H


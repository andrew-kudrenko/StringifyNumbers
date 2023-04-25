#include <iostream>
#include <fstream>
#include <regex>
#include <vector>

using converter_fn = std::string (std::string, int);

const int DIGITS_COUNT = 10;

const int DIGIT_GROUP_LENGTH = 3;

const int DIGIT_GROUPS_LIMIT = 4;

const int FRACTIONAL_ORDER_LIMIT = 6;

std::string fractional_order_names[FRACTIONAL_ORDER_LIMIT] = {
    "tenths",
    "hundredths",
    "thousandths",
    "ten thousandths",
    "hundred thousandth",
    "millionths"
};

std::string digit_group_names[DIGIT_GROUPS_LIMIT] = {
    "",
    "thousand",
    "million",
    "billion",
};

std::string basics[DIGITS_COUNT] = {
    "zero",
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
};

std::string tens[DIGITS_COUNT] = {
    "",
    "ten",
    "twenty",
    "thirty",
    "fourty",
    "fifty",
    "sixty",
    "seventy",
    "eighty",
    "ninety",
};

std::string first_ten[DIGITS_COUNT] = {
    "",
    "eleven",
    "twelve",
    "thirteen",
    "fourteen",
    "fifteen",
    "sixteen",
    "seventeen",
    "eighteen",
    "nineteen",
};

std::string require_target_filepath(std::string prelude)
{
    std::cout << prelude << " ";
    
    std::string filepath;
    std::cin >> filepath;

    return filepath;
}

bool is_word_number(std::string word)
{
    return std::regex_match(word, std::regex("^(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?$"));
}

bool is_word_in_range(std::string word)
{
    int length = word.length();
    const int LIMIT = 6;

    return length > 0 && length < LIMIT;
}

int digit_to_int(char symbol)
{
    return (int)symbol - 48;
}

std::vector<std::string> *split(std::string text, char delimiter)
{
    auto accumulator = new std::vector<std::string>();
    int take_from = 0;

    for (int idx = 0; idx < text.length(); idx++)
    {
        if (text[idx] == delimiter || idx == text.length() - 1)
        {
            accumulator->push_back(text.substr(take_from, idx));
            take_from = idx + 1;
        }
    }

    return accumulator;
}

std::string convert_single_digit_word(std::string word, int at)
{
    if (word[at] == '0' && word.length() > 1)
    {
        return "";
    }

    int prev = at - 1;

    if (prev >= 0 && word[prev] == '1')
    {
        return "";
    }

    return basics[digit_to_int(word[at])];
}

std::string convert_tens_word(std::string word, int at)
{
    int tens_digit = digit_to_int(word[at]);
    int base_digit = digit_to_int(word[static_cast<char>(at + 1)]);

    bool is_inside_first_ten = tens_digit == 1 && base_digit > 0;

    return is_inside_first_ten ? first_ten[base_digit] : tens[tens_digit];
}

std::string convert_hundreds_word(std::string word, int at)
{
    int hundreds = digit_to_int(word[at]);

    return hundreds ? basics[hundreds] + " hundred" : "";
}

converter_fn* get_converter(int at)
{
    switch (at)
    {
    case 0:
        return convert_single_digit_word;
    case 1:
        return convert_tens_word;
    case 2:
        return convert_hundreds_word;
    default:
        return NULL;
    }
}

std::string add_group_name(std::string word, int group_idx)
{
    if (word.empty())
    {
        return word;
    }

    return digit_group_names[group_idx].empty() ? word : word + " " + digit_group_names[group_idx];
}

std::string add_fractional_part_name(std::string number, int order)
{
    if (order > 0 && order <= FRACTIONAL_ORDER_LIMIT) {
        return number + " " + fractional_order_names[order - 1];
    }

    return "";
}

std::string stringify_group(std::string word, int group_idx)
{   
    std::string result = "";
    int start_group_idx = (int)word.length() - group_idx * DIGIT_GROUP_LENGTH;

    for (int idx = 0; idx <= DIGIT_GROUP_LENGTH; idx++)
    {
        int order = DIGIT_GROUP_LENGTH - idx - 1;
        converter_fn* convert_with = get_converter(order);
    
        if (convert_with != NULL)
        {
            int at = start_group_idx - DIGIT_GROUP_LENGTH + idx;
        
            if (at < 0)
            {
                continue;
            
            }
            
            std::string converted = convert_with(word, at);
            
            if (!converted.empty())
            {
                if (!result.empty() && result.back() != ' ')
                {
                    result += ' ';
                }

                result += converted;
            }
        }
    }

    return add_group_name(result, group_idx);
}

std::string convert_word_to_integer(std::string word)
{
    std::string result = "";
    int groups_count = word.length() / DIGIT_GROUP_LENGTH;

    for (int idx = groups_count; idx >= 0; idx--)
    {
        std::string converted_group = stringify_group(word, idx);

        if (!converted_group.empty())
        {
            if (!result.empty())
            {
                result += " ";
            }

            result += converted_group;
        }
    }

    return result;
}

std::string convert_word_to_number(std::string word)
{
    std::string result = "";
    auto parts = split(word, '.');

    if (parts->size() == 1)
    {
        return convert_word_to_integer(word);
    }

    std::string fractional = add_fractional_part_name(convert_word_to_integer(parts->at(1)), parts->at(1).size());

    return convert_word_to_integer(parts->at(0)) + " dot " + fractional;
}

void forward_words(std::ifstream* from, std::ofstream* to)
{
    std::string word;

    while (*from >> word)
    {
        std::string converted_word = is_word_number(word) ? word + " (" + convert_word_to_number(word) + ")" : word;
        *to << converted_word << std::endl;
    }
}

int main()
{
    std::cout << "Please enter paths where you want to load from and save in.\n\n";

    std::string filepath = require_target_filepath("Filepath>");
    std::string target_filepath = require_target_filepath("Target Filepath>");

    try {
        std::ifstream from;
        from.open(filepath);

        std::ofstream to;
        to.open(target_filepath);

        if (from.is_open() && to.is_open())
        {
            std::cout << "Files successfully opened!" << std::endl;
        }

        forward_words(&from, &to);

        std::cout << "Done!" << std::endl;

        from.close();
        to.close();
    }
    catch (std::exception exception) 
    {
        std::cout << "Something went wrong" << std::endl;
    }

    return 0;
}

#include "argparse.hpp"
#include "fmt/core.h"

#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "fmts.hpp"

#include <iostream>
#include <fstream>

std::vector<miniplc0::Token> _tokenize(std::istream& input) {
	miniplc0::Tokenizer tkz(input);
	auto p = tkz.AllTokens();
	if (p.second.has_value()) {
		fmt::print(stderr, "Tokenization error: {}\n", p.second.value());
		exit(2);
	}
	return p.first;
}

void Tokenize(std::istream& input, std::ostream& output) {
	auto v = _tokenize(input);
	for (auto& it : v)
		output << fmt::format("{}\n", it);
	return;
}

void Analyse(std::istream& input, std::ostream& output,std::string s){
	auto tks = _tokenize(input);
	miniplc0::Analyser analyser(tks);
	auto p = analyser.Analyse(s,output );
	if (p.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
		exit(2);
	}
	auto v = p.first;
	for (auto& it : v)
		output << fmt::format("{}\n", it);
	return;
}
//
//void TranslateIntoAssembleFile(std::istream& input, std::ostream& output){
//    auto tks = _tokenize(input);
//    miniplc0::Analyser analyser(tks);
//    auto p = analyser.Analyse("-s",output);
//    if (p.second.has_value()) {
//        fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
//        exit(2);
//    }
//    auto v = p.first;
//    for (auto& it : v)
//        output << fmt::format("{}\n", it);
//    return;
//}
//
//void TranslateIntoBinaryFile(std::istream& input, std::ostream& output){
//    auto tks = _tokenize(input);
//    miniplc0::Analyser analyser(tks);
//    auto p = analyser.Analyse("-c", output);
//    if (p.second.has_value()) {
//        fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
//        exit(2);
//    }
//    auto v = p.first;
//    for (auto& it : v)
//        output << fmt::format("{} \n\n", it);
//    return;
//}

int main(int argc, char** argv) {
	argparse::ArgumentParser program("cc0");
	program.add_argument("input")
		.help("speicify the file to be compiled.");
	program.add_argument("-t")
		.default_value(false)
		.implicit_value(true)
		.help("perform tokenization for the input file.");
	program.add_argument("-l")
		.default_value(false)
		.implicit_value(true)
		.help("perform syntactic analysis for the input file.");
    program.add_argument("-s")
        .default_value(false)
        .implicit_value(true)
        .help("translate the input C0 source code into a text assembly file.");
    program.add_argument("-c")
            .default_value(false)
            .implicit_value(true)
            .help("translate the input C0 source code into binary target file.");
	program.add_argument("-o", "--output")
		.required()
		.default_value(std::string("-"))
		.help("specify the output file.");


	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		fmt::print(stderr, "{}", err.what());
		program.print_help();
		exit(2);
	}
/*cc0 [options] input [-o file]
  cc0 [-h]*/
	auto input_file = program.get<std::string>("input");
	auto output_file = program.get<std::string>("--output");
	std::istream* input;
	std::ostream* output;
	std::ifstream inf;
	std::ofstream outf;
    if (input_file != "-") {
        inf.open(input_file, std::ios::binary | std::ios::in);
        if (!inf) {
            fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
            exit(2);
        }
        input = &inf;
    }
    else
        input = &std::cin;
    if (output_file != "-") {
        outf.open(output_file, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!outf) {
            fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
            exit(2);
        }
        output = &outf;
    }
    else
        output = &std::cout;
	if (program["-t"] == true && program["-l"] == true) {
		fmt::print(stderr, "You can only perform tokenization or syntactic analysis at one time.");
		exit(2);
	}
	if (program["-t"] == true) {
		Tokenize(*input, *output);
	}
	else if (program["-l"] == true) {
		Analyse(*input, *output,"-l");
	}
	else if (program["-s"] == true){
	    Analyse(*input,*output,"-s");
	}
    else if (program["-c"] == true) {
        Analyse(*input,*output,"-c");
    }
	else {
		fmt::print(stderr, "You must choose tokenization or syntactic analysis.");
		exit(2);
	}
	return 0;
}

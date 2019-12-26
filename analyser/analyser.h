#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"

#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace miniplc0 {

	class Analyser final {
	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
	public:
		Analyser(std::vector<Token> v)
			: _tokens(std::move(v)), _offset(0), _instructions({}), _current_pos(0, 0),
			_uninitialized_vars({}), _vars({}), _consts({}), _nextTokenIndex(0) {}
		Analyser(Analyser&&) = delete;
		Analyser(const Analyser&) = delete;
		Analyser& operator=(Analyser) = delete;

		// 唯一接口
        std::pair<std::vector<Instruction> , std::optional<CompilationError>> Analyse(std::string s,std::ostream& output);
	private:
		// 所有的递归子程序

		// <C0>
		std::optional<CompilationError> analyseProgram();
		// <variable-declaration>
        std::optional<CompilationError> analyseVariableDeclaration();
        //<init-declarator-list>
        std::optional<CompilationError> analyseInitDeclarationList();
        //<init-declarator>
        std::optional<CompilationError> analyseInitDeclarator();
        //<Expression>
        std::optional<CompilationError> analyseExpression();
        //<analyseAddiveExpression>
        std::optional<CompilationError> analyseAddiveExpression();
        //<analyseMultiplicativeEexpression()>
        std::optional<CompilationError> analyseMultiplicativeExpression();
        //analyseCastExpression()
        std::optional<CompilationError> analyseCastExpression();
        //analyseUnaryExpression()
        std::optional<CompilationError> analyseUnaryExpression();
        //analysePrimaryExpression()
        std::optional<CompilationError> analysePrimaryExpression();
        //analyseFunctionCall()
        std::optional<CompilationError> analyseFunctionCall();
        //analyseExpressionList()
        std::optional<CompilationError> analyseExpressionList();
        //analyseFunctionDefinition();
        std::optional<CompilationError> analyseFunctionDefinition();
        //analyseParameterClause()
        std::optional<CompilationError> analyseParameterClause();
        //analyseParameterDeclarationList()
        std::optional<CompilationError> analyseParameterDeclarationList();
        //analyseParameterDeclaration()
        std::optional<CompilationError> analyseParameterDeclaration();
        //analyseCompoundStatement
        std::optional<CompilationError> analyseCompoundStatement();
        //analyseStatementSeq()
        std::optional<CompilationError> analyseStatementSeq();
        //analyseStatement()
        std::optional<CompilationError> analyseStatement();
        //analyseConditionStatement()
        std::optional<CompilationError> analyseConditionStatement();
        //analyseCondition()
        std::optional<CompilationError> analyseCondition();
        //analyseLoopStatement()
        std::optional<CompilationError> analyseLoopStatement();
        //analyseReturnStatement()
        std::optional<CompilationError> analyseReturnStatement();
        //analysePrintStatement()
        std::optional<CompilationError> analysePrintStatement();
        //analysePrintableList()
        std::optional<CompilationError> analysePrintableList();
        //analyseScanStatement
        std::optional<CompilationError> analyseScanStatement();
        //analyseAssignmentExpression()
        std::optional<CompilationError> analyseAssignmentExpression();
        //将指令结构体添加
        void addFunIns(std::string,bool x1,int x,bool y1,int y,std::string fn,uint16_t op);
        //输出二进制文件
        void output_binary(std::ostream& out);
        // Token 缓冲区相关操作
		// 返回下一个 token
		std::optional<Token> nextToken();
		// 回退一个 token
		void unreadToken();

		// 下面是符号表相关操作

		// helper function
		void _add(const Token&, std::map<std::string, int32_t>&);
		// 添加变量、常量、未初始化的变量
		void addVariable(std::string s,int type);
		void addConstant(std::string s,int type);
		void addUninitializedVariable(std::string s,int type);
		void addFunctionName(std::string s,int returnType,int paraNum);
		// 是否被声明过
		bool isDeclared(const std::string&s);
		bool globalIsDeclared(const std::string&);
		// 是否是未初始化的变量
		bool isUninitializedVariable(const std::string&);
        bool globalUninitializedVariable(const std::string&s);
		// 是否是已初始化的变量
		bool isInitializedVariable(const std::string& );
		bool globalIsInitializedVariable(const std::string&s);
		// 是否是常量
		bool isConstant(const std::string&);
		bool globalIsConstant(const std::string&s);
		//是否是函数
		bool isFunction(const std::string&s);
		// 获得 {变量，常量} 在栈上的偏移
		int getIndex(std::string s);
        int globalGetIndex(std::string s);
        //设置opcode中参数所占单元
        struct insStruct getParaSize(struct insStruct ss);
        //或者指令在指令集中的位置
        int getInstructionIndex(const std::string&s);
	private:
		std::vector<Token> _tokens;
		std::size_t _offset;
		std::vector<Instruction> _instructions;
		std::pair<uint64_t, uint64_t> _current_pos;

		// 为了简单处理，我们直接把符号表耦合在语法分析里
		// 变量                   示例
		// _uninitialized_vars    var a;
		// _vars                  var a=1;
		// _consts                const a=1;
		std::map<std::string, int32_t> _uninitialized_vars;
		std::map<std::string, int32_t> _vars;
		std::map<std::string, int32_t> _consts;
		// 下一个 token 在栈的偏移
		int32_t _nextTokenIndex;
		//函数在函数表中的偏移
		int32_t _functionIndex = 0;
	};
}

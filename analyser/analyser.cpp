#include "analyser.h"
#include "assert.h"
#include <iostream>
#include <fstream>
#include <climits>
#include <list>
#include <map>
#include <string>
#include <cstring>
#include <algorithm>


namespace miniplc0 {

    bool isConstVar = false;
    int variableType;
    int _paraNum = 0;
    //记录函数中Token位置
    struct tokenPlace{
        std::string name;
        int32_t place;
    };
    std::map< std::string,std::vector<tokenPlace>> _tokenPlace;

    //当前解析的函数名
    std::string funName = "global";
    //function符号表，包含初始化变量和未初始化变量
    typedef struct variableStruct{
        std::string name;
        int type;
        int paraNum;
    } ;
    struct funIdentiferList{
        //常量表
        std::vector<variableStruct> constList;
        //未声明变量表
        std::vector<variableStruct> noIniVariableList;
        //常量已声明
        std::vector<variableStruct> iniVariableList;
        //对于全局部分，会有函数名
        std::vector<variableStruct> _funName;
    };
    std::map<std::string ,funIdentiferList> funList;

    //指令集，每个函数有一个栈
    std::map<std::string ,std::vector<std::string>> funInstructionList ;
    //指令结构体集
    struct insStruct{
        std::string insName;
        uint16_t op;
        bool hasParaX = false;
        bool hasParaY = false;
        uint32_t x;
        uint32_t y;
        int sizeofX;
        int sizeofY;
    };
    std::map<std::string ,std::vector<insStruct>> funInstructionStruct;

    //提供一个全局变量，分析调用参数数目
    int functionCall_paraNum;

    //参数1--指令，参数2--输出文件
	std::pair<std::vector<Instruction> , std::optional<CompilationError>> Analyser::Analyse(std::string s,std::ostream& output) {
		auto err = analyseProgram();
		if (err.has_value())
            return std::make_pair(std::vector<Instruction>(), err);
		//在此返回指令集
		else if(s == "-s"){
		    //输出格式
            //fmt::format("{}\n", it);
            //.constants:
		    std::string startSign = ".constants:";
            output << startSign <<'\n';
            //只把函数作为常量
            for(int i = 0 ; i < funList["global"]._funName.size(); i++){
                std::string inf = std::to_string(i) + " " + "S" + " "  + "\""+ funList["global"]._funName[i].name + "\"";
                output << inf << '\n';
            }

            //.start
            startSign = ".start:";
            output << startSign << '\n';
            for(int i = 0 ; i < funInstructionList["global"].size(); i++){
                std::string inf = std::to_string(i)+" "+ funInstructionList["global"][i].c_str();
                output << inf << '\n';
            }

            //.functions
            startSign = ".functions:";
            output << startSign << '\n';
            for(int i = 0 ; i < funList["global"]._funName.size(); i++){
                std::string inf = std::to_string(i)+" "+std::to_string(i)+" "+std::to_string(funList["global"]._funName[i].paraNum)+" "+"1";
                output << inf << '\n';
            }

            //每个函数的具体指令
            for(int i = 0 ; i < funList["global"]._funName.size(); i++){
                std::string funName = funList["global"]._funName[i].name;
                std::string F = ".F"+std::to_string(i)+":";
                output << F << '\n';
                for(int j = 0 ; j < funInstructionList[funName].size(); j++){
                    std::string inf = std::to_string(j) + " " + funInstructionList[funName][j];
                    output << inf << '\n';
                }
            }
            return std::make_pair(_instructions, std::optional<CompilationError>());
		}
		else if(s == "-c"){
//		    printf("entrance\n");
//		    for(int i = 0 ; i < funList["global"]._funName.size();i++){
//		        std::string fn = funList["global"]._funName[i].name;
//		        for(int j = 0 ; j < funInstructionStruct["main"].size();j++){
//		            struct insStruct ss = funInstructionStruct[fn][j];
//		            printf("%s %d ",ss.insName.c_str(),ss.op);
//		            if(ss.hasParaX == true)
//		                printf("%d ",ss.x);
//                    if(ss.hasParaY == true)
//                        printf("%d ",ss.y);
//		            printf("\n");
//		        }
//		    }
		    //printf("12121");
		    output_binary((std::ofstream &) output);
            return std::make_pair(_instructions, std::optional<CompilationError>());
		}
        return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <C0-program> ::={<variable-declaration>}{<function-definition>}
	std::optional<CompilationError> Analyser::analyseProgram() {
	    while(true){
            auto bg = nextToken();
            if(!bg.has_value()) {
                return {};
            }
            if(bg.value().GetType() == TokenType::CONST){
                unreadToken();
                //变量声明分析
                auto err = analyseVariableDeclaration();
                if(err.has_value())
                    return err;
            }
            else if(bg.value().GetType() == TokenType::VOID){
                //说明是函数定义，跳出循环
                unreadToken();
                break;
            }
            else if(bg.value().GetType() == TokenType::INT ||
                    bg.value().GetType() == TokenType::CHAR ||
                    bg.value().GetType() == TokenType::DOUBLE){
                bg = nextToken();
                if(bg.value().GetType()!= TokenType::IDENTIFIER){
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidInput);
                }
                else{
                    bg = nextToken();
                    if(bg.value().GetType() == TokenType::LEFT_BRACKET){
                        unreadToken();//回退（
                        unreadToken();//回退identifer
                        unreadToken();//回退关键字
                        break;
                    }
                    else{
                        /*printf("this variableDefine");*/
                        unreadToken();
                        unreadToken();
                        unreadToken();
                        auto err = analyseVariableDeclaration();
                        if(err.has_value())
                            return err;
                    }

                }
            }
            else
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidInput);
	    }
	    //函数声明
	    while(true){
	        auto bg = nextToken();
	        if(!bg.has_value())
                break;
	        unreadToken();
            auto err = analyseFunctionDefinition();
            if(err.has_value()){
                return err;
            }
	    }
	    //判断是否有main作为入口
	    for(int i = 0 ;i < funList["global"]._funName.size() ;i++){
	        if(funList["global"]._funName[i].name == "main"){
	            return {};
	        }
	    }
        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEntrance);
	}

	//<variable-declaration> ::=[<const-qualifier>]<type-specifier><init-declarator-list>';'
    std::optional<CompilationError> Analyser::analyseVariableDeclaration(){
	    auto next = nextToken();
	    if(next.value().GetType() == TokenType::CONST){
	        isConstVar = true;
	    }
	    else
	        unreadToken();
	    next = nextToken();
	    /*std::string s = next.value().GetValueString();
	    std::cout << s << std::endl;*/
	    switch(next.value().GetType()){
	        case TokenType::INT:
	            variableType = TokenType::INT;
	            break;
            default:
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrTypeSpecifier);
	    }
	    auto err = analyseInitDeclarationList();
	    if(err.has_value())
	        return err;
        next = nextToken();
        if(next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        //重置不是常量
        isConstVar = false;
        return{};
	}

    //<init-declarator-list> ::=<init-declarator>{','<init-declarator>}
    std::optional<CompilationError> Analyser::analyseInitDeclarationList(){
	    auto err = analyseInitDeclarator();
	    if(err.has_value())
	        return err;
	    while(true){
	        auto next = nextToken();
	        if(!next.has_value()){
                return {};
	        }
	        if(next.value().GetType() != TokenType::EXCLAMATION){
	            unreadToken();
	            return {};
	        }
            err = analyseInitDeclarator();
	        if(err.has_value())
	            return err;
	    }
	}

	//这里出现了IDENTIFER要根据IDENTIFER所在位置将IDENTIFET添加到符号表表中
    //<init-declarator> ::=<identifier>[<initializer>]
    //<initializer> ::='='<expression>
    std::optional<CompilationError> Analyser::analyseInitDeclarator(){
        auto next = nextToken();
        if(next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidIdentifier);
        //std::cout<<next.value().GetType() << "++"<<next.value().GetValueString();
        std::string name = next.value().GetValueString();
        //是否声明过,这个只用在当前变量表中查找
        if(isDeclared(name))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

        //常量先添加到常量表中
        if(isConstVar){
            printf("1c");
            addConstant(name,variableType);
        }
        //预读
        next = nextToken();
        //std::cout<<next.value().GetType() << "++"<<next.value().GetValueString()<<std::endl;
        if(next.value().GetType() == TokenType::EQUAL_SIGN){
            //添加到初始化表中
            auto err = analyseExpression();
            if(err.has_value())
                return err;
            if(!isConstVar)
                addVariable(name,variableType);
        }
        else{
             //常量不赋值报错？
             if(isConstVar)
                 return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);
             //变量没有赋值
             addUninitializedVariable(name,variableType);
             //不是常量且没有赋值的话，压一个数站位
             std::string ins = "ipush 1";
             funInstructionList[funName].push_back(ins);
             //ipush--0x02
             addFunIns("ipush",true,1, false,-1,funName,0x02);
            unreadToken();
        }
        return {};
	}

    // <expression> ::=<additive-expression>
    std::optional<CompilationError> Analyser::analyseExpression() {
        auto err = analyseAddiveExpression();
        if(err.has_value())
            return err;
        return {};
    }

    //<additive-expression> ::=<multiplicative-expression>{<additive-operator><multiplicative-expression>}
    std::optional<CompilationError> Analyser::analyseAddiveExpression(){
	    auto err = analyseMultiplicativeExpression();
	    if(err.has_value())
	        return err;
	    while(true){
	        auto next = nextToken();
	        if(!next.has_value()){
	            return{};
	        }
	        if(next.value().GetType() != TokenType::MINUS_SIGN && next.value().GetType() != TokenType::PLUS_SIGN){
	            unreadToken();
	            return{};
	        }
	        auto type = next.value().GetType();
	        err = analyseMultiplicativeExpression();
	        if(err.has_value())
	            return err;
	        std::string instruction;
	        if(type == TokenType::PLUS_SIGN){
	            instruction = "iadd" ;
	            funInstructionList[funName].push_back(instruction);
	            //iadd -- 0x30
                addFunIns("iadd",false,-1, false,-1,funName,0x30);
	        }
	        if(type == TokenType::MINUS_SIGN){
                instruction = "isub" ;
                funInstructionList[funName].push_back(instruction);
                //isub -- 0x34
                addFunIns("isub",false,-1, false,-1,funName,0x34);
	        }
	    }
	}

     //<multiplicative-expression> ::=<unary-expression>{<multiplicative-operator><unary-expression>}
    std::optional<CompilationError> Analyser::analyseMultiplicativeExpression(){
            auto err = analyseUnaryExpression();
            if(err.has_value())
                return err;
            while(true){
                auto next = nextToken();
                if(!next.has_value())
                    return {};
                if(next.value().GetType() != TokenType::MULTIPLICATION_SIGN &&
                    next.value().GetType() != TokenType::DIVISION_SIGN){
                    unreadToken();
                    return {};
                }
                auto type = next.value().GetType();
                err = analyseUnaryExpression();
                if(err.has_value())
                    return err;
                std::string instruction;
                if(type == TokenType::MULTIPLICATION_SIGN){
                    instruction = "imul";
                    funInstructionList[funName].push_back(instruction);
                    //imul -- 0x38
                    addFunIns("imul",false,-1, false,-1,funName,0x38);
                }
                if(type == TokenType::DIVISION_SIGN){
                    instruction = "idiv";
                    funInstructionList[funName].push_back(instruction);
                    //idiv -- 0x3c
                    addFunIns("idiv",false,-1, false,-1,funName,0x3c);
                }
            }
	}

    //<cast-expression> ::={'('<type-specifier>')'}<unary-expression>
    std::optional<CompilationError> Analyser::analyseCastExpression(){
	    while(true){
            auto next = nextToken();
            if(!next.has_value())
                break;
            if(next.value().GetType() != TokenType::LEFT_BRACKET){
                unreadToken();
                break;
            }
            next = nextToken();
            switch(next.value().GetType()){
                case TokenType::VOID:
                    break;
                case TokenType::CHAR:
                    break;
                case TokenType::INT:
                    break;
                case TokenType::DOUBLE:
                    break;
                default:
                    printf("12");
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrTypeSpecifier);
            }
            next = nextToken();
            if(next.value().GetType() != TokenType::RIGHT_BRACKET)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRightBracket);
	    }
	    auto err = analyseUnaryExpression();
	    if(err.has_value())
	        return err;
	    return{};
	}

    //<unary-expression> ::=[<unary-operator>]<primary-expression>
    std::optional<CompilationError> Analyser::analyseUnaryExpression(){
	    auto next = nextToken();
	    if(next.value().GetType() != TokenType::PLUS_SIGN &&
	        next.value().GetType() != TokenType::MINUS_SIGN)
	        unreadToken();
        auto unaryFlag = next.value().GetType();//给后面的用
	    auto err = analysePrimaryExpression();
	    if(err.has_value())
	        return err;
        if(unaryFlag == TokenType::MINUS_SIGN){
            funInstructionList[funName].push_back("ineg");
            //ineg -- 0x40
            addFunIns("ineg",false,-1, false,-1,funName,0x40);
        }
	    return {};
	}

    //<primary-expression> ::= '('<expression>')'
    //    |<identifier> --finish
    //    |<integer-literal> --finish
    //    |<char-literal> --waiting
    //    |<floating-literal> --waiting
    //    |<function-call>
    std::optional<CompilationError> Analyser::analysePrimaryExpression(){
	    auto next = nextToken();
	    if(next.value().GetType() != TokenType::LEFT_BRACKET &&
	       next.value().GetType() != TokenType::UNSIGNED_INTEGER &&
	       next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidInput);
	    switch (next.value().GetType()) {
            case TokenType::LEFT_BRACKET:{
                auto err = analyseExpression();
                if (err.has_value())
                    return err;
                next = nextToken();
                if (next.value().GetType() != TokenType::RIGHT_BRACKET)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRightBracket);
                return {};
            }
            case TokenType::IDENTIFIER:{
                std::string copy1 = next.value().GetValueString();
                next = nextToken();
                //是(，分析函数调用
                if (next.value().GetType() == TokenType::LEFT_BRACKET) {
                    unreadToken();//将（回退
                    unreadToken();//将IDENTIFER回退

                    //判断作为表达式的函数
                    std::string tmp = funName;
                    funName = "global";
                    int funIndex = getIndex(copy1);
                    //printf("\n%s--%d\n",next.value().GetValueString().c_str(),funList["global"]._funName[funIndex].type);
                    if(funList["global"]._funName[funIndex].type == TokenType::VOID){
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoReturn);
                    }
                    funName = tmp;

                    auto err = analyseFunctionCall();
                    if (err.has_value())
                        return err;
                    return {};
                }
                //不是，解析IDENTIFER
                unreadToken();
                unreadToken();
                next = nextToken(); //此时读到标识符IDENTIFER
                std::string str = next.value().GetValueString();
                std::string ins;
                //首先判断是否为局部变量
                if(isDeclared(str)){
                    //是否初始化过或者是常量
                    if(isConstant(str)){
                       // std::cout<<str<<getIndex(str)<<std::endl;
                        //加载IDENTIFER值
                        ins = "loada 0,"+ std::to_string(getIndex(str));
                        funInstructionList[funName].push_back(ins);
                        //loada -- 0x0a
                        addFunIns("loada",true,0, true,getIndex(str),funName,0x0a);
                        //存储IDENTIFER的值给新的IDENTIFER
                        ins = "iload";
                        funInstructionList[funName].push_back(ins);
                        //iload (0x10)
                        addFunIns("iload",false,-1, false,-1,funName,0x10);
                    }
                    else if(isInitializedVariable(str)){
                       // std::cout<<str<<getIndex(str)<<std::endl;
                        //加载IDENTIFER值
                        ins = "loada 0,"+std::to_string(getIndex(str)) ;
                        funInstructionList[funName].push_back(ins);
                        //loada -- 0x0a
                        addFunIns("loada",true,0, true,getIndex(str),funName,0x0a);

                        //存储IDENTIFER的值给新的IDENTIFER
                        ins = "iload";
                        funInstructionList[funName].push_back(ins);
                        //iload (0x10)
                        addFunIns("iload",false,-1, false,-1,funName,0x10);
                    }
                    else
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
                }
                //如果是全局声明的变量
                else if(globalIsDeclared(str)){
                    if(globalIsConstant(str) ||globalIsInitializedVariable(str)){
                        //加载IDENTIFER值
                        ins = "loada 1,"+std::to_string(globalGetIndex(str)) ;
                        funInstructionList[funName].push_back(ins);
                        //loada -- 0x0a
                        addFunIns("loada",true,1, true,globalGetIndex(str),funName,0x0a);
                        //存储IDENTIFER的值给新的IDENTIFER
                        ins = "iload";
                        funInstructionList[funName].push_back(ins);
                        //iload (0x10)
                        addFunIns("iload",false,-1, false,-1,funName,0x10);
                    }
                    else
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
                }
                else{
                    printf("this is 2");
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
                }

                return {};
            }
            case TokenType::UNSIGNED_INTEGER:{
                //这里只用于函数调用时
                //解析无符号数，无符号整数入栈
                std::string ins = "ipush "  + std::to_string(std::any_cast<int>(next.value().GetValue()));
               // std::cout<<ins;
                funInstructionList[funName].push_back(ins);
                //ipush -- 0x02
                addFunIns("ipush", true,std::any_cast<int>(next.value().GetValue()), false,-1,funName,0x02);
                return {};
            }
            default:
                break;
        }
        return{};
	}


    //<function-call> ::=<identifier> '(' [<expression-list>] ')
    std::optional<CompilationError> Analyser::analyseFunctionCall() {
        //调用参数数目为0
        functionCall_paraNum = 0;
	    bool if_has_para = false;
	    auto next = nextToken();
	    //读到IDENTIFER
	    if(!isFunction(next.value().GetValueString())){
	        //printf("there:%s",next.value().GetValueString().c_str());
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunNoDeclared);
	    }

	    std::string functionName = next.value().GetValueString();
	    next = nextToken();
	    if(next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLeftBracket);
	    next = nextToken();
	    if(next.value().GetType() != TokenType::RIGHT_BRACKET){
	        if_has_para = true;
	        unreadToken();
            auto err = analyseExpressionList();
            if(err.has_value())
                return err;
            //参数调用时
            std::string tmp = funName;
            funName = "global";
            int funIndex = getIndex(functionName);
            if(functionCall_paraNum != funList["global"]._funName[funIndex].paraNum){
                printf("\nfun:%d -- para:%d\n",functionCall_paraNum,funList["global"]._funName[funIndex].paraNum);
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrWrongParaNum);
            }
            funName = tmp;

            next = nextToken();
            if(next.value().GetType() != TokenType::RIGHT_BRACKET)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRightBracket);
           // printf("121");
	    }
	    std::string ins = "call " + std::to_string(globalGetIndex(functionName));
	    funInstructionList[funName].push_back(ins);
        //call -- (0x80)
        addFunIns("call", true,globalGetIndex(functionName), false,-1,funName,0x80);
//	    if(if_has_para){
//	       ins = "pop";
//	       funInstructionList[funName].push_back(ins);
//            //pop (0x04)
//            addFunIns("pop", false,-1, false,-1,funName,0x04);
//	    }
	    return {};
	}

    //<expression-list> ::=<expression>{','<expression>}
    std::optional<CompilationError> Analyser::analyseExpressionList() {
        functionCall_paraNum++;
	    auto err = analyseExpression();
	    if(err.has_value())
	        return err;
	    while(true){
	        auto next = nextToken();
	        if(!next.has_value())
	            return {};
	        if(next.value().GetType() != TokenType::EXCLAMATION){
	            unreadToken();
	            return {};
	        }
	        err = analyseExpression();
            functionCall_paraNum++;
	        if(err.has_value())
	            return err;
	    }
	}

	//<function-definition> ::=<type-specifier><identifier><parameter-clause><compound-statement>
    std::optional<CompilationError> Analyser::analyseFunctionDefinition() {
	    int returnType ;
	    auto next = nextToken();
	    if(next.value().GetType() != TokenType::VOID  &&
	        next.value().GetType() != TokenType::INT){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrTypeSpecifier);
	    }
	    returnType = next.value().GetType();
	    //获取返回值类型
	    auto type = next.value().GetType();
	    next = nextToken();
	    if(next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoIdentifier);
        std::string fName = next.value().GetValueString();
        //判断函数名是否被定义过
	    if(globalIsDeclared(fName))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
        /*std::cout<<fName<<type;*/
	    //分析当前函数定义，更改分析的函数名
	    funName = fName;
	    _nextTokenIndex = 0;
        _paraNum = 0;
	    auto err = analyseParameterClause();
	    if(err.has_value())
	        return err;
	    //分析完参数数目，将全局参数值添加
	   // printf("==%s==%d==%d",fName.c_str(),type,_paraNum);
        addFunctionName(fName,type,_paraNum);
	    //设置参数
	    err = analyseCompoundStatement();
	    if(err.has_value())
	        return err;
	    if(returnType == TokenType::INT ){
            //压入一个返回语句，防止函数没有返回
            std::string ins = "iret";
            funInstructionList[funName].push_back(ins);
            //ret (0x88)
            addFunIns("iret", false,-1, false,-1,funName,0x89);
	    }
	    if(returnType == TokenType::VOID ){
            //压入一个返回语句，防止函数没有返回
            std::string ins = "ret";
            funInstructionList[funName].push_back(ins);
            //ret (0x88)
            addFunIns("ret", false,-1, false,-1,funName,0x88);
	    }
	    //完成函数分析，退出分析前将分析范围改为全局
	    //测试
	    //printf("\n%s\n",fName.c_str());
	    //for(int i = 0 ; i < funInstructionList[funName].size();i++)
	     // printf("%s\n",funInstructionList[funName][i].c_str());
	    //函数范围回到全局
	    funName = "global";
	    //_nextTokenPlace恢复到下标指针处
	    _nextTokenIndex = _tokenPlace["global"].size();

	    return {};

	}

	//<parameter-clause> ::='(' [<parameter-declaration-list>] ')'
    std::optional<CompilationError> Analyser::analyseParameterClause(){
        auto next = nextToken();
        if(next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLeftBracket);
	    next = nextToken();
        if(next.value().GetType() == TokenType::RIGHT_BRACKET){
            return{};
        }
        else{
            unreadToken();
            auto err = analyseParameterDeclarationList();
            if(err.has_value())
                return err;
            next = nextToken();
            if(next.value().GetType() != TokenType::RIGHT_BRACKET)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRightBracket);
            return {};
        }
    }

    //<parameter-declaration-list> ::=<parameter-declaration>{','<parameter-declaration>}
    std::optional<CompilationError> Analyser::analyseParameterDeclarationList(){
	    auto err = analyseParameterDeclaration();
	    if(err.has_value())
	        return err;
	    while(true){
	        auto next = nextToken();
	        if(!next.has_value())
	            return {};
	        if(next.value().GetType() != TokenType::EXCLAMATION){
	            unreadToken();
	            return {};
	        }
	        err = analyseParameterDeclaration();
	        if(err.has_value())
	            return err;
	    }
	}

	//<parameter-declaration> ::=[<const-qualifier>]<type-specifier><identifier>
    std::optional<CompilationError> Analyser::analyseParameterDeclaration(){
	    auto next = nextToken();
	    if(next.value().GetType() == TokenType::CONST){
	        isConstVar = true;
            next = nextToken();
	    }
	    //参数分析中不可能有VOID类型
	    switch(next.value().GetType()){
	        case TokenType::INT:
	            variableType=TokenType::INT;
	            break;
            default:
                //printf("14");
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrTypeSpecifier);
        }

        next = nextToken();
	    if(next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoIdentifier);
	    //判断是否重复声明，不用判断全局
	    if(isDeclared(next.value().GetValueString()))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
	    if(isConstVar){
	        //参数入表
	       // printf("2c");
	        addConstant(next.value().GetValueString(),variableType);
	    }
	    else{
            addVariable(next.value().GetValueString(),variableType);
	    }
	    //参数数目+1
	    _paraNum++;
//        std::string ins = "ipush 0";
//        funInstructionList[funName].push_back(ins);
//        //ipush(0x02)
//        addFunIns("ipush", true,0, false,-1,funName,0x02);

        //重新置为默认非常量
        isConstVar =false;
	    return {};
    }

    //<compound-statement> ::='{' {<variable-declaration>} <statement-seq> '}'
    std::optional<CompilationError> Analyser::analyseCompoundStatement(){
        auto next = nextToken();
        if(next.value().GetType() != TokenType::LEFT_BIG_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLeftBigBracket);
        while(true){
            auto next = nextToken();
            if(!next.has_value())
                break;
            if(next.value().GetType() != TokenType::CONST &&
                next.value().GetType() != TokenType::VOID &&
                next.value().GetType() != TokenType::INT
               ){
                unreadToken();
                break;
            }
            unreadToken();
            auto err = analyseVariableDeclaration();
            if(err.has_value())
                return err;
        }
        auto err = analyseStatementSeq();
        if(err.has_value())
            return err;
        next = nextToken();
        if(next.value().GetType() != TokenType::RIGHT_BIG_BRACKET){
            printf("this is 1:%s",next.value().GetValueString().c_str());
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRightBigBracket);
        }

        return {};
    }

    //<statement-seq> ::={<statement>}
    //<statement> ::='{' <statement-seq> '}'
    //    |<condition-statement>
    //    |<loop-statement>
    //    |<jump-statement>
    //    |<print-statement>
    //    |<scan-statement>
    //    |<assignment-expression>';'
    //    |<function-call>';'
    //    |';'
    std::optional<CompilationError> Analyser::analyseStatementSeq(){
	    while(true){
	        //直接分析statementSeq
	        auto next = nextToken();
	        if(!next.has_value())
	            return{};
	        //???????
	        switch(next.value().GetType()){
	            case TokenType::LEFT_BIG_BRACKET :{
                    auto err = analyseStatementSeq();
                    if(err.has_value())
                        return err;
                    next = nextToken();
                    if(next.value().GetType() != TokenType::RIGHT_BIG_BRACKET){
                        printf("this is 3");
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRightBigBracket);
                    }

                    break;
	            }
	            case TokenType::IF :{
	                unreadToken();
	                auto err = analyseConditionStatement();
	                if(err.has_value())
	                    return err;
                    break;
	            }
	            case TokenType::WHILE :{
	                unreadToken();
	                auto err = analyseLoopStatement();
	                if(err.has_value())
	                    return err;
	                break;
	            }
	            case TokenType::RETURN :{
	                unreadToken();
	                auto err = analyseReturnStatement();
	                if(err.has_value())
	                    return err;
	                break;
	            }
	            case TokenType::PRINT :{
	                unreadToken();
	                auto err = analysePrintStatement();
	                if(err.has_value())
	                    return err;
	                break;
	            }
	            case TokenType::SCAN :{
	                unreadToken();
	                auto err = analyseScanStatement();
	                if(err.has_value())
	                    return err;
	                break;
	            }
	            case TokenType::IDENTIFIER :{
                    //printf("%s",next.value().GetValueString().c_str());
	                next = nextToken();
	                if(next.value().GetType() == TokenType::EQUAL_SIGN){
	                    unreadToken();
	                    unreadToken();
	                    auto err = analyseAssignmentExpression();
	                    if(err.has_value())
	                        return err;
	                }
	                else if(next.value().GetType() == TokenType::LEFT_BRACKET){
	                    printf("This 3");
	                    unreadToken();
	                    unreadToken();
	                    auto err = analyseFunctionCall();
	                    if(err.has_value())
	                        return err;
	                }
	                else
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidInput);
	                next = nextToken();
	                if(next.value().GetType() != TokenType::SEMICOLON)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
                    break;
                }
                case TokenType::SEMICOLON:
                    break;
                default:
                    unreadToken();
                    return{};
	        }
	    }
	}

    //<statement> ::='{' <statement-seq> '}'
    //    |<condition-statement>
    //    |<loop-statement>
    //    |<jump-statement>
    //    |<print-statement>
    //    |<scan-statement>
    //    |<assignment-expression>';'
    //    |<function-call>';'
    //    |';'
    std::optional<CompilationError> Analyser::analyseStatement(){
        auto next = nextToken();
        switch(next.value().GetType()){
            case TokenType::LEFT_BIG_BRACKET :{
                auto err = analyseStatementSeq();
                if(err.has_value())
                    return err;
                next = nextToken();
                if(next.value().GetType() != TokenType::RIGHT_BIG_BRACKET){
                    printf("this is 2");
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRightBigBracket);
                }
                break;
            }
            case TokenType::IF :{
                unreadToken();
                auto err = analyseConditionStatement();
                if(err.has_value())
                    return err;
                break;
            }
            case TokenType::WHILE :{
                unreadToken();
                auto err = analyseLoopStatement();
                if(err.has_value())
                    return err;
                break;
            }
            case TokenType::RETURN :{
                unreadToken();
                auto err = analyseReturnStatement();
                if(err.has_value())
                    return err;
                break;
            }
            case TokenType::PRINT :{
                unreadToken();
                auto err = analysePrintStatement();
                if(err.has_value())
                    return err;
                break;
            }
            case TokenType::SCAN :{
                unreadToken();
                auto err = analyseScanStatement();
                if(err.has_value())
                    return err;
                break;
            }
            case TokenType::IDENTIFIER :{
                next = nextToken();
                if(next.value().GetType() == TokenType::EQUAL_SIGN){
                    unreadToken();
                    unreadToken();
                    auto err = analyseAssignmentExpression();
                    if(err.has_value())
                        return err;
                }
                else if(next.value().GetType() == TokenType::LEFT_BRACKET){
                    printf("this 2");
                    unreadToken();
                    unreadToken();
                    auto err = analyseFunctionCall();
                    if(err.has_value())
                        return err;
                }
                else
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidInput);
                next = nextToken();
                if(next.value().GetType() != TokenType::SEMICOLON)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
                break;
            }
            case TokenType::SEMICOLON:
                break;
            default:
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidInput);
        }
        return{};
	}

	int conditionType = -1;
	//<condition-statement> ::='if' '(' <condition> ')' <statement> ['else' <statement>]
    std::optional<CompilationError> Analyser::analyseConditionStatement(){
	    auto next = nextToken();
        if(next.value().GetType() != TokenType::IF)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoIF);
        next = nextToken();
	    if(next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLeftBracket);
	    auto err = analyseCondition();
	    //存当前获得的condition
	    int con = conditionType;

	    if(err.has_value())
	        return err;
	    next = nextToken();
	    if(next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRightBracket);
        //程序要么从这里进入
        std::string place = "IF_FIRST_ENTRY";
        funInstructionList[funName].push_back(place);
        addFunIns(place, false,-1, false,-1,funName,-1);
	    err = analyseStatement();
        if(err.has_value())
            return err;
        //程序要么从这里进入
        place = "IF_SECOND_ENTRY";
        funInstructionList[funName].push_back(place);
        addFunIns(place, false,-1, false,-1,funName,-1);
        next = nextToken();
        if(next.value().GetType() == TokenType::ELSE){
            err = analyseStatement();
            if(err.has_value())
                return err;
        }
        else
            unreadToken();
        //设置跳转指令
        int if_first_entry = getInstructionIndex("IF_FIRST_ENTRY"),
        if_second_entry = getInstructionIndex("IF_SECOND_ENTRY");
        switch(con){
                case TokenType::SMALL_SIGN:
                    //jge offset(2) (0x74);
                    funInstructionList[funName][if_first_entry] = "jge " +std::to_string((if_second_entry+1));
                    funInstructionStruct[funName][if_first_entry].insName="jge" , funInstructionStruct[funName][if_first_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_first_entry].x = if_second_entry+1;funInstructionStruct[funName][if_first_entry].op = 0x74;
                    //jmp offset(2) (0x70)
                    funInstructionList[funName][if_second_entry] = "jmp " + std::to_string((funInstructionList[funName].size() ));
                    funInstructionStruct[funName][if_second_entry].insName="jmp" , funInstructionStruct[funName][if_second_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_second_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][if_second_entry].op = 0x70;
                    break;
                case TokenType::SMALL_EQUAL_SIGN:
                    //jg offset(2) (0x75)
                    funInstructionList[funName][if_first_entry] = "jg " +std::to_string((if_second_entry+1));
                    funInstructionStruct[funName][if_first_entry].insName="jg" , funInstructionStruct[funName][if_first_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_first_entry].x = if_second_entry+1;funInstructionStruct[funName][if_first_entry].op = 0x75;

                    funInstructionList[funName][if_second_entry] = "jmp " + std::to_string((funInstructionList[funName].size() ));
                    funInstructionStruct[funName][if_second_entry].insName="jmp" , funInstructionStruct[funName][if_second_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_second_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][if_second_entry].op = 0x70;
                    break;
                case TokenType::MORE_SIGN:
                    //jle offset(2) (0x76)
                    funInstructionList[funName][if_first_entry] = "jle " +std::to_string((if_second_entry+1));
                    funInstructionStruct[funName][if_first_entry].insName="jle" , funInstructionStruct[funName][if_first_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_first_entry].x = if_second_entry+1;funInstructionStruct[funName][if_first_entry].op = 0x76;

                    funInstructionList[funName][if_second_entry] = "jmp " + std::to_string((funInstructionList[funName].size() ));
                    funInstructionStruct[funName][if_second_entry].insName="jmp" , funInstructionStruct[funName][if_second_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_second_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][if_second_entry].op = 0x70;
                    break;
                case TokenType::MORE_EQUAL_SIGN:
                    //jl offset(2) (0x73)
                    funInstructionList[funName][if_first_entry] = "jl " +std::to_string((if_second_entry+1));
                    funInstructionStruct[funName][if_first_entry].insName="jl" , funInstructionStruct[funName][if_first_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_first_entry].x = if_second_entry+1;funInstructionStruct[funName][if_first_entry].op = 0x73;

                    funInstructionList[funName][if_second_entry] = "jmp " + std::to_string(funInstructionList[funName].size() );
                    funInstructionStruct[funName][if_second_entry].insName="jmp" , funInstructionStruct[funName][if_second_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_second_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][if_second_entry].op = 0x70;
                    break;
                case TokenType::EQUAL_EQUAL_Sign:
                    //jne offset(2) (0x72)
                    funInstructionList[funName][if_first_entry] = "jne " +std::to_string((if_second_entry+1));
                    funInstructionStruct[funName][if_first_entry].insName="jne" , funInstructionStruct[funName][if_first_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_first_entry].x = if_second_entry+1;funInstructionStruct[funName][if_first_entry].op = 0x72;

                    funInstructionList[funName][if_second_entry] = "jmp " + std::to_string(funInstructionList[funName].size() );
                    funInstructionStruct[funName][if_second_entry].insName="jmp" , funInstructionStruct[funName][if_second_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_second_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][if_second_entry].op = 0x70;
                    break;
                case TokenType::NOT_EQUAL_SIGN:
                    //je (0x71)
                    funInstructionList[funName][if_first_entry] = "je " +std::to_string((if_second_entry+1));
                    funInstructionStruct[funName][if_first_entry].insName="je" , funInstructionStruct[funName][if_first_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_first_entry].x = if_second_entry+1;funInstructionStruct[funName][if_first_entry].op = 0x71;

                    funInstructionList[funName][if_second_entry] = "jmp " + std::to_string(funInstructionList[funName].size() );
                    funInstructionStruct[funName][if_second_entry].insName="jmp" , funInstructionStruct[funName][if_second_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_second_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][if_second_entry].op = 0x70;
                    break;
                default:

                    funInstructionList[funName][if_first_entry] = "je " +std::to_string((if_second_entry+1));
                    funInstructionStruct[funName][if_first_entry].insName="je" , funInstructionStruct[funName][if_first_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_first_entry].x = if_second_entry+1;funInstructionStruct[funName][if_first_entry].op = 0x71;

                    funInstructionList[funName][if_second_entry] = "jmp " + std::to_string(funInstructionList[funName].size() );
                    funInstructionStruct[funName][if_second_entry].insName="jmp" , funInstructionStruct[funName][if_second_entry].hasParaX = true ;
                    funInstructionStruct[funName][if_second_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][if_second_entry].op = 0x70;
                    break;
        }
        //恢复默认
        //conditionType = -1;
        return {};
    }

    //<condition> ::=<expression>[<relational-operator><expression>]
    std::optional<CompilationError> Analyser::analyseCondition(){
	    auto err = analyseExpression();
	    if(err.has_value())
	        return err;
	    auto next = nextToken();

        if(next.value().GetType() != TokenType::SMALL_SIGN &&
            next.value().GetType() != TokenType::SMALL_EQUAL_SIGN &&
            next.value().GetType() != TokenType::EQUAL_EQUAL_Sign &&
            next.value().GetType() != TokenType::MORE_SIGN &&
            next.value().GetType() != TokenType::MORE_EQUAL_SIGN &&
            next.value().GetType() != TokenType::NOT_EQUAL_SIGN){
            unreadToken();
            //与0比较
            std::string cmp0 = "ipush 0";
            funInstructionList[funName].push_back(cmp0);
            addFunIns("ipush",true,0,false,-1,funName,0x02);
            cmp0 = "icmp";
            funInstructionList[funName].push_back(cmp0);//(0x44)
            addFunIns("icmp", false,-1,false,-1,funName,0x44);
            conditionType = -1;
        }
        else{
            printf("\n%s\n",next.value().GetValueString().c_str());
            //比较符号
            int type = next.value().GetType();
            err =analyseExpression();
            if(err.has_value())
                return err;
            //将栈顶两个值比较
            std::string ins = "icmp";
            funInstructionList[funName].push_back(ins);
            addFunIns("icmp", false,-1,false,-1,funName,0x44);
            //满足条件时顺序进行，不跳转
            switch(type) {
                case TokenType::SMALL_SIGN:
                    conditionType = TokenType::SMALL_SIGN;
                    break;
                case TokenType::SMALL_EQUAL_SIGN:
                    conditionType = TokenType::SMALL_EQUAL_SIGN;
                    break;
                case TokenType::MORE_SIGN:
                    conditionType = TokenType::MORE_SIGN;
                    break;
                case TokenType::MORE_EQUAL_SIGN:
                    conditionType = TokenType::MORE_EQUAL_SIGN;
                    break;
                case TokenType::EQUAL_EQUAL_Sign:
                    conditionType = TokenType::EQUAL_EQUAL_Sign;
                    break;
                case TokenType::NOT_EQUAL_SIGN:
                    conditionType = TokenType::NOT_EQUAL_SIGN;
                    break;
                default:
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConditionType);
                }
        }
        return {};
	}

	//<loop-statement> ::='while' '(' <condition> ')' <statement>
    std::optional<CompilationError> Analyser::analyseLoopStatement(){
	    auto next = nextToken();
        if(next.value().GetType() != TokenType::WHILE)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoWhile);
        next = nextToken();
	    if(next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLeftBracket);
	    //循环体地址开始处,此处是condition第一条指令
	    int while_start_index = funInstructionList[funName].size();

	    auto err = analyseCondition();
	    //存分析获得conditionType
	    if(err.has_value())
	        return err;
        int con = conditionType;

	    next = nextToken();
	    if(next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRightBracket);

	    funInstructionList[funName].push_back("WHILE_ENTRY");
        addFunIns("WHILE_ENTRY", false,-1, false,-1,funName,-1);
        err = analyseStatement();
        if(err.has_value())
            return err;

        funInstructionList[funName].push_back("WHILE_END");
        addFunIns("WHILE_END", false,-1, false,-1,funName,-1);
        int while_entry = getInstructionIndex("WHILE_ENTRY") ,
        while_end = getInstructionIndex("WHILE_END");
        //printf("~~%d~~",conditionType);
        switch(con){
            case TokenType::SMALL_SIGN: {
                funInstructionList[funName][while_end] = "jmp " + std::to_string(while_start_index);// (0x70)
                funInstructionStruct[funName][while_end].insName="jmp" , funInstructionStruct[funName][while_end].hasParaX = true ;
                funInstructionStruct[funName][while_end].x = while_start_index ; funInstructionStruct[funName][while_end].op = 0x70;

                funInstructionList[funName][while_entry] = "jge " +std::to_string(funInstructionList[funName].size());//(0x74)
                funInstructionStruct[funName][while_entry].insName="jge" , funInstructionStruct[funName][while_entry].hasParaX = true ;
                funInstructionStruct[funName][while_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][while_entry].op = 0x74;
                break;
            }
            case TokenType::SMALL_EQUAL_SIGN:
                funInstructionList[funName][while_end] = "jmp " + std::to_string(while_start_index);// (0x70)
                funInstructionStruct[funName][while_end].insName="jmp" , funInstructionStruct[funName][while_end].hasParaX = true ;
                funInstructionStruct[funName][while_end].x = while_start_index ; funInstructionStruct[funName][while_end].op = 0x70;

                funInstructionList[funName][while_entry] = "jg " +std::to_string(funInstructionList[funName].size());//0x75
                funInstructionStruct[funName][while_entry].insName="jg" , funInstructionStruct[funName][while_entry].hasParaX = true ;
                funInstructionStruct[funName][while_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][while_entry].op = 0x75;
                break;
            case TokenType::MORE_SIGN:
                funInstructionList[funName][while_end] = "jmp " + std::to_string(while_start_index);// (0x70)
                funInstructionStruct[funName][while_end].insName="jmp" , funInstructionStruct[funName][while_end].hasParaX = true ;
                funInstructionStruct[funName][while_end].x = while_start_index ; funInstructionStruct[funName][while_end].op = 0x70;

                funInstructionList[funName][while_entry] = "jle " +std::to_string(funInstructionList[funName].size());//0x76
                funInstructionStruct[funName][while_entry].insName="jle" , funInstructionStruct[funName][while_entry].hasParaX = true ;
                funInstructionStruct[funName][while_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][while_entry].op = 0x76;
                break;
            case TokenType::MORE_EQUAL_SIGN:
                funInstructionList[funName][while_end] = "jmp " + std::to_string(while_start_index);// (0x70)
                funInstructionStruct[funName][while_end].insName="jmp" , funInstructionStruct[funName][while_end].hasParaX = true ;
                funInstructionStruct[funName][while_end].x = while_start_index ; funInstructionStruct[funName][while_end].op = 0x70;

                funInstructionList[funName][while_entry] = "jl " +std::to_string(funInstructionList[funName].size());//0x73
                funInstructionStruct[funName][while_entry].insName="jl" , funInstructionStruct[funName][while_entry].hasParaX = true ;
                funInstructionStruct[funName][while_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][while_entry].op = 0x73;
                break;
            case TokenType::EQUAL_EQUAL_Sign:
                funInstructionList[funName][while_end] = "jmp " + std::to_string(while_start_index);// (0x70)
                funInstructionStruct[funName][while_end].insName="jmp" , funInstructionStruct[funName][while_end].hasParaX = true ;
                funInstructionStruct[funName][while_end].x = while_start_index ; funInstructionStruct[funName][while_end].op = 0x70;

                funInstructionList[funName][while_entry] = "jne " +std::to_string(funInstructionList[funName].size());//0x72
                funInstructionStruct[funName][while_entry].insName="jne" , funInstructionStruct[funName][while_entry].hasParaX = true ;
                funInstructionStruct[funName][while_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][while_entry].op = 0x72;
                break;
            case TokenType::NOT_EQUAL_SIGN:
                funInstructionList[funName][while_end] = "jmp " + std::to_string(while_start_index);// (0x70)
                funInstructionStruct[funName][while_end].insName="jmp" , funInstructionStruct[funName][while_end].hasParaX = true ;
                funInstructionStruct[funName][while_end].x = while_start_index ; funInstructionStruct[funName][while_end].op = 0x70;

                funInstructionList[funName][while_entry] = "je " +std::to_string(funInstructionList[funName].size());//0x71
                funInstructionStruct[funName][while_entry].insName="je" , funInstructionStruct[funName][while_entry].hasParaX = true ;
                funInstructionStruct[funName][while_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][while_entry].op = 0x71;
                break;
            default:
                funInstructionList[funName][while_end] = "jmp " + std::to_string(while_start_index);// (0x70)
                funInstructionStruct[funName][while_end].insName="jmp" , funInstructionStruct[funName][while_end].hasParaX = true ;
                funInstructionStruct[funName][while_end].x = while_start_index ; funInstructionStruct[funName][while_end].op = 0x70;

                funInstructionList[funName][while_entry] = "je " +std::to_string(funInstructionList[funName].size());//0x71
                funInstructionStruct[funName][while_entry].insName="je" , funInstructionStruct[funName][while_entry].hasParaX = true ;
                funInstructionStruct[funName][while_entry].x = funInstructionList[funName].size();funInstructionStruct[funName][while_entry].op = 0x71;
                break;
        }
        conditionType = -1;
        return {};
    }

    //<return-statement> ::= 'return' [<expression>] ';'
    std::optional<CompilationError> Analyser::analyseReturnStatement(){
	    auto next = nextToken();
        if(next.value().GetType() != TokenType::RETURN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoReturn);

        next = nextToken();
        //不是分号，分析表达式
        if(next.value().GetType() != TokenType::SEMICOLON){
            //判断函数是否有返回值，没有报错
            std::string tmp = funName;
            funName = "global";
            int funIndex = getIndex(tmp);
            if(funList["global"]._funName[funIndex].type == TokenType::VOID){
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrHasReturn);
            }
            funName = tmp;

            unreadToken();
            auto err = analyseExpression();
            if(err.has_value())
                return err;
            next = nextToken();
            if(next.value().GetType() != TokenType::SEMICOLON)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
            std::string ins = "iret";//(0x89)
            funInstructionList[funName].push_back(ins);
            addFunIns("iret", false,-1,false,-1,funName,0x89);
        }
        //是分号
        else{
            //检查函数是否有返回值
            std::string tmp = funName;
            funName = "global";
            if(funList[funName]._funName[getIndex(tmp)].type == TokenType::INT)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoReturn);
            funName = tmp;

            std::string ins = "ret";
            funInstructionList[funName].push_back(ins);//(0x88)
            addFunIns("ret", false,-1,false,-1,funName,0x88);
        }
        return {};
    }

    //<print-statement> ::= 'print' '(' [<printable-list>] ')' ';'
    std::optional<CompilationError> Analyser::analysePrintStatement(){
	    auto next = nextToken();
	    if(next.value().GetType() != TokenType::PRINT)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoPrint);
        next = nextToken();
	    if(next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLeftBracket);
	    next = nextToken();
	    if(next.value().GetType() != TokenType::RIGHT_BRACKET){
	        unreadToken();
	        auto err = analysePrintableList();
	        if(err.has_value())
	            return err;
	        next = nextToken();
	        if(next.value().GetType() != TokenType::RIGHT_BRACKET)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRightBracket);
	        next = nextToken();
            if(next.value().GetType() != TokenType :: SEMICOLON)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        }
	    else{
	        next = nextToken();
	        if(next.value().GetType() != TokenType :: SEMICOLON)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        }
        //压入换行
        std::string ins = "bipush 10";// 换行/r
        funInstructionList[funName].push_back(ins);
        addFunIns("bipush", true,10,false,-1,funName,0x01);
        //打印换行
        ins = "cprint";//iprint (0xa0)
        funInstructionList[funName].push_back(ins);
        addFunIns("cprint", false,-1,false,-1,funName,0xa2);
        return {};
    }

    //<printable-list>  ::= <printable> {',' <printable>}
    //<printable> ::= <expression>
    std::optional<CompilationError> Analyser::analysePrintableList(){
	    auto err = analyseExpression();
	    if(err.has_value())
	        return err;
	    //在这里添加打印语句
        //有值打印
        std::string ins = "iprint";//iprint (0xa0)
        funInstructionList[funName].push_back(ins);
        addFunIns("iprint", false,-1,false,-1,funName,0xa0);
	    while(true){
	        auto next = nextToken();
	        if(!next.has_value())
	            return{};
	        if(next.value().GetType() != TokenType::EXCLAMATION){
	            unreadToken();
	            return {};
	        }
	        //压入空格
	        ins = "bipush 32";
            funInstructionList[funName].push_back(ins);
            addFunIns("bipush", true,' ',false,-1,funName,0x01);
            //打印空格
            ins = "cprint";//iprint (0xa0)
            funInstructionList[funName].push_back(ins);
            addFunIns("cprint", false,-1,false,-1,funName,0xa2);

	        err = analyseExpression();
	        if(err.has_value())
	            return err;
	        //执行打印内容
            ins = "iprint";//iprint (0xa0)
            funInstructionList[funName].push_back(ins);
            addFunIns("iprint", false,-1,false,-1,funName,0xa0);
	    }
	}

	//<scan-statement>  ::= 'scan' '(' <identifier> ')' ';'
    std::optional<CompilationError> Analyser::analyseScanStatement(){
	    auto next = nextToken();
	    if(next.value().GetType() != TokenType::SCAN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoScan);

        next = nextToken();
	    if(next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLeftBracket);

	    next = nextToken();
        if(next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoIdentifier);

        std::string str = next.value().GetValueString();
        //变量是否声明过
        if(!isDeclared(str) && !globalIsConstant(str) && !globalUninitializedVariable(str) && !globalIsInitializedVariable(str) ){
            printf("this is 3");
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
        }
        //要求不是常量
        if(isConstant(str) || (!isUninitializedVariable(str) && !isInitializedVariable(str) && globalIsConstant(str)) ){
            printf("thissss fault");
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
        }
        std::string ins;
        if(isUninitializedVariable(str) || isInitializedVariable(str)){
            //未初始化的先初始化
            if(!isInitializedVariable(str))
                addVariable(str,TokenType::INT);
            ins = "loada 0," + std::to_string(getIndex(str));
            addFunIns("loada", true,0,true,getIndex(str),funName,0x0a);
            funInstructionList[funName].push_back(ins);
        }
        else if(globalUninitializedVariable(str) || globalIsInitializedVariable(str)){
            if(!globalIsInitializedVariable(str)){
                std::string tmp = funName;
                funName = "global";
                addVariable(str,TokenType::INT);
                funName = tmp;
            }
            ins = "loada 1," + std::to_string(globalGetIndex(str));
            funInstructionList[funName].push_back(ins);
            addFunIns("loada", true,1,true,globalGetIndex(str),funName,0x0a);
        }
        else
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
        next = nextToken();
        if(next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRightBracket);
        next = nextToken();
        if(next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        ins = "iscan";
        funInstructionList[funName].push_back(ins);//iscan (0xb0)
        addFunIns("iscan", false,-1,false,-1,funName,0xb0);
        //取出栈顶的值
        //压入IDENTIFER地址
        ins = "istore";//istore (0x20)
        funInstructionList[funName].push_back(ins);
        addFunIns("istore", false,-1,false,-1,funName,0x20);
        return {};
    }

    //<assignment-expression> ::=<identifier><assignment-operator><expression>
    std::optional<CompilationError> Analyser::analyseAssignmentExpression(){
	    auto next = nextToken();
	    if(next.value().GetType() != TokenType::IDENTIFIER)
	        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoIdentifier);
	   std::string str = next.value().GetValueString();
	    //有没有声明过
	    if(!isDeclared(str) && !globalUninitializedVariable(str) && !isInitializedVariable(str)){
	        printf("this is 1");
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
	    }


	    //不能对常量赋值
	    if(isConstant(str) || (!isUninitializedVariable(str) && !isInitializedVariable(str) && globalIsConstant(str)) ){
            printf("this fault");
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
	    }
        std::string ins;
        bool isGlobalVar ;
        //局部
        if(isUninitializedVariable(str) || isInitializedVariable(str)){
            ins = "loada 0,"  + std::to_string(getIndex(str));
            addFunIns("loada", true,0,true,getIndex(str),funName,0x0a);
            funInstructionList[funName].push_back(ins);
            isGlobalVar = false;
        }
            //全局
        else if(globalUninitializedVariable(str) || globalIsInitializedVariable(str)){
            ins = "loada 1,"  + std::to_string(globalGetIndex(str));//(0x0a)
            funInstructionList[funName].push_back(ins);
            addFunIns("loada", true,1,true,globalGetIndex(str),funName,0x0a);
            isGlobalVar = true;
        }

	    next = nextToken();
	    if(next.value().GetType() != TokenType::EQUAL_SIGN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
	    auto err = analyseExpression();
	    if(err.has_value())
	        return err;

	    if(isGlobalVar){
            std::string tmp_fun_name = funName;
            funName = "global";
            addVariable(str,TokenType::INT);
            funName = tmp_fun_name;
	    }
	    else{
            addVariable(str,TokenType::INT);
	    }

        ins = "istore";
        funInstructionList[funName].push_back(ins);
        addFunIns("istore", false,-1,false,-1,funName,0x20);
        return {};
    }


	std::optional<Token> Analyser::nextToken() {
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() {
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}


	void Analyser::addVariable(std::string s,int type) {
        struct variableStruct vb;
        vb.name = s, vb.type = type;
        funList[funName].iniVariableList.push_back(vb);
        struct tokenPlace tp;
        tp.name = s;
        tp.place = _nextTokenIndex;
        _tokenPlace[funName].push_back(tp);
        _nextTokenIndex++;
	}

	void Analyser::addConstant(std::string s,int type) {
        struct variableStruct vb;
        vb.name = s, vb.type = type;
        funList[funName].constList.push_back(vb);
        struct tokenPlace tp;
        tp.name = s;
        tp.place = _nextTokenIndex;
       // std::cout<<_nextTokenIndex;
        _tokenPlace[funName].push_back(tp);
        _nextTokenIndex++;
	}

	void Analyser::addUninitializedVariable(std::string s,int type) {
	    //添加到表中
	    struct variableStruct vb;
	    vb.name = s, vb.type = type;
	    funList[funName].noIniVariableList.push_back(vb);
        struct tokenPlace tp;
        tp.name = s;
        tp.place = _nextTokenIndex;
        //std::cout<<_nextTokenIndex;
        _tokenPlace[funName].push_back(tp);
        _nextTokenIndex++;
	}

    void Analyser::addFunctionName(std::string s, int returnType,int paraNum)  {
        //添加到表中
        struct variableStruct vb;
        vb.name = s, vb.type = returnType, vb.paraNum = paraNum;
        //在全局表中添加
        funList["global"]._funName.push_back(vb);
        struct tokenPlace tp;
        tp.name = s;
        tp.place = _functionIndex;
        _tokenPlace["global"].push_back(tp);
        printf("_fun:%d",_functionIndex);
        _functionIndex++;
    }

	int32_t Analyser::getIndex(std::string s) {
        for(int i = 0 ; i < _tokenPlace[funName].size() ;i++){
            if(s == _tokenPlace[funName][i].name)
                return _tokenPlace[funName][i].place;
        }
        return -1;
	}

	bool Analyser::isDeclared(const std::string& s) {
		return isConstant(s) || isUninitializedVariable(s) || isInitializedVariable(s) || isFunction(s);
	}

	bool Analyser::isUninitializedVariable(const std::string& s) {
        for(int i = 0; i < funList[funName].noIniVariableList.size(); i++){
            if(s == funList[funName].noIniVariableList[i].name)
                return true;
        }
        return false;

	}
	bool Analyser::isInitializedVariable(const std::string&s) {
        for(int i = 0 ;i < funList[funName].iniVariableList.size() ;i++){
            if(s == funList[funName].iniVariableList[i].name)
                return true;
        }
        return false;
	}

	bool Analyser::isConstant(const std::string&s) {
	    for(int i = 0 ; i < funList[funName].constList.size() ;i++){
	        if(s == funList[funName].constList[i].name){
                printf("--%s--",s.c_str());
	            return true;
	        }

	    }
        return false;
	}
	//找全局变量位置
	int Analyser::globalGetIndex(std::string s){
        for(int i = 0 ; i < _tokenPlace["global"].size() ;i++){
            if(s == _tokenPlace["global"][i].name){
                return _tokenPlace["global"][i].place;
            }
        }
        return -1;
	}

	//针对全局内容的判断
	bool Analyser::globalIsDeclared(const std::string&s){
	    return globalIsConstant(s) || globalIsInitializedVariable(s) || globalUninitializedVariable(s) || isFunction(s);
	}

	bool Analyser::globalUninitializedVariable(const std::string&s){
        for(int i = 0 ; i < funList["global"].noIniVariableList.size() ;i++){
            if(s == funList["global"].noIniVariableList[i].name)
                return true;
        }
        return false;
	}

	bool Analyser::globalIsInitializedVariable(const std::string&s){
        for(int i = 0 ; i < funList["global"].iniVariableList.size() ;i++){
            if(s == funList["global"].iniVariableList[i].name)
                return true;
        }
        return false;
	}

	bool Analyser::globalIsConstant(const std::string&s){
        for(int i = 0 ; i < funList["global"].constList.size() ;i++){
            if(s == funList["global"].constList[i].name)
                return true;
        }
        return false;
	}

	bool Analyser::isFunction(const std::string &s) {
	    for(int i = 0 ; i < funList["global"]._funName.size() ;i++){
	        printf("--%s--%d--%d--",funList["global"]._funName[i].name.c_str(),funList["global"]._funName[i].type,funList["global"]._funName[i].paraNum);
	        if(funList["global"]._funName[i].name == s)
	            return true;
	    }
	    return false;
	}

	int Analyser::getInstructionIndex(const std::string&s){
	    for(int i = 0 ;i < funInstructionList[funName].size(); i++){
	        if(funInstructionList[funName][i] == s)
	            return i;
	    }
	    return -1;
	}

	void Analyser::addFunIns(std::string name,bool x1,int x,bool y1,int y,std::string fn,uint16_t op){
            struct insStruct aStruct;
            aStruct.insName = name;
            aStruct.hasParaX = x1,
            aStruct.hasParaY = y1;
            aStruct.x = x;
            aStruct.y = y;
            aStruct.op = op;
            funInstructionStruct[fn].push_back(aStruct);
    }

	void Analyser::output_binary(std::ostream& out){
        char bytes[8];
        const auto writeNBytes = [&](void* addr, int count) {
            assert(0 < count && count <= 8);
            char* p = reinterpret_cast<char*>(addr) + (count-1);
            for (int i = 0; i < count; ++i) {
                bytes[i] = *p--;
            }
            out.write(bytes, count);
        };
        std::string s ;
        // magic
        out.write("\x43\x30\x3A\x29", 4);
         true;
        // version
        out.write("\x00\x00\x00\x01", 4);

        //const_count
        uint16_t constants_count = funList["global"]._funName.size();
        writeNBytes(&constants_count, sizeof(constants_count));
        //constants
        for(int i = 0 ; i < funList["global"]._funName.size();i++){
            out.write("\x00",1);//类型string
            //长度lenth
            std::string v = funList["global"]._funName[i].name;
            uint16_t len = v.length();
            writeNBytes(&len, sizeof(len));
            //名称+名称长度
            out.write(v.c_str(), len);
        }

        //start_code
        uint16_t start_ins_count = funInstructionStruct["global"].size();
        writeNBytes(&start_ins_count, sizeof (start_ins_count));
        struct insStruct insS,ss;
        for(int i = 0 ; i < funInstructionStruct["global"].size();i++){
            insS = funInstructionStruct["global"][i];
            ss = getParaSize(insS);
            writeNBytes(&insS.op, sizeof(uint8_t));//指令
            if(insS.hasParaX == true) //参数1
                writeNBytes(&insS.x, ss.sizeofX);
            if(insS.hasParaY == true) //参数2
                writeNBytes(&insS.y, ss.sizeofY);
        }
        //function_count
        uint16_t functions_count = funList["global"]._funName.size();
        writeNBytes(&functions_count, sizeof(functions_count));

        //functions
        for(int i = 0 ; i < funList["global"]._funName.size();i++){
            std::string fn = funList["global"]._funName[i].name;//函数名
            uint16_t v = i;
            writeNBytes(&v, sizeof(uint16_t));//name_index
            writeNBytes(&funList["global"]._funName[i].paraNum, sizeof(uint16_t));//para_Lenth
            v = 1;
            writeNBytes(&v, sizeof(uint16_t));//level
            v = funInstructionList[fn].size();
            writeNBytes(&v,sizeof(uint16_t));//ins_count
            for(int j = 0 ; j < funInstructionStruct[fn].size();j++){
                insS = funInstructionStruct[fn][j];
////                printf("%s %d ",insS.insName.c_str(),insS.op);
////                if(insS.hasParaX == true)
////                    printf("%d ",insS.x);
////                if(insS.hasParaY == true)
////                    printf("%d ",insS.y);
////                printf("\n");
////                if(insS.insName == "loada")
////                    out.write("\x0a",1);
////                else
                writeNBytes(&insS.op, sizeof(uint8_t));//指令
 //               printf("==%s==%d==\n",insS.insName.c_str(),insS.op);
                ss = getParaSize(insS);
                if(insS.hasParaX == true) //参数1
                    writeNBytes(&insS.x, ss.sizeofX);
                if(insS.hasParaY == true) //参数2
                    writeNBytes(&insS.y, ss.sizeofY);
            }
        }
    }

    struct insStruct Analyser::getParaSize(struct insStruct ss){
        std::string ins = ss.insName;
        if(ins == "ipush"){
            ss.op = 0x02;
            ss.sizeofX = 4;
        }
        else if(ins == "loada"){
            ss.op = 0x0a;
            ss.sizeofX = 2;
            ss.sizeofY = 4;
        }
        else if(ins == "jmp"){
            ss.op = 0x70;
            ss.sizeofX = 2;
        }
        else if(ins == "je"){
            ss.op = 0x71;
            ss.sizeofX = 2;
        }
        else if(ins == "jne"){
            ss.op = 0x72;
            ss.sizeofX = 2;
        }
        else if(ins == "jl"){
            ss.op = 0x73;
            ss.sizeofX = 2;
        }
        else if(ins == "jge"){
            ss.op = 0x74;
            ss.sizeofX = 2;
        }
        else if(ins == "jg"){
            ss.op = 0x75;
            ss.sizeofX = 2;
        }
        else if(ins == "jle"){
             ss.op = 0x76;
             ss.sizeofX = 2;
        }
        else if(ins == "call"){
            ss.op = 0x80;
            ss.sizeofX = 2;
        }
        else if(ins == "bipush"){
            ss.op = 0x01;
            ss.sizeofX = 1;
        }
        return ss;
    }
}
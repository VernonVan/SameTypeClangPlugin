#include <iostream>
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
using namespace clang;
using namespace std;
using namespace llvm;
using namespace clang::ast_matchers;

namespace SameTypePlugin
{    
    class SameTypeHandler : public MatchFinder::MatchCallback
    {
    private:
        CompilerInstance &Instance;
        ASTContext *context;
        std::vector<ObjCPropertyDecl *> propertyDeclVector;
        
    public:
        SameTypeHandler(CompilerInstance &Instance) : Instance(Instance) {}
        
        void setContext(ASTContext &context)
        {
            this->context = &context;
        }
        
        virtual void run(const MatchFinder::MatchResult &Result)
        {
            if (const ObjCPropertyDecl *propertyDecl = Result.Nodes.getNodeAs<ObjCPropertyDecl>("objcPropertyDecl")) {
                if (isUserSourceDecl(propertyDecl)) {
                    // 存储 Objective-C 类属性
                    propertyDeclVector.push_back(const_cast<ObjCPropertyDecl *>(propertyDecl));
                }
            } else if (const BinaryOperator *binaryOperator = Result.Nodes.getNodeAs<BinaryOperator>("binaryOperator")) {
                if (isUserSourceStmt(binaryOperator) && binaryOperator->isAssignmentOp()) {
                    ObjCPropertyRefExpr *leftExpr = dyn_cast_or_null<ObjCPropertyRefExpr>(binaryOperator->getLHS());
                    OpaqueValueExpr *rightExpr = dyn_cast_or_null<OpaqueValueExpr>(binaryOperator->getRHS());
                    if (leftExpr && rightExpr) {
                        std::string propertyName = leftExpr->getGetterSelector().getAsString();
                        if (ImplicitCastExpr *castExpr = dyn_cast_or_null<ImplicitCastExpr>(rightExpr->getSourceExpr())) {
                            if (ObjCMessageExpr *messageExpr = dyn_cast_or_null<ObjCMessageExpr>(castExpr->getSubExpr())) {
                                ObjCMethodDecl *methodDecl = messageExpr->getMethodDecl();
                                if (checkIfHasAttribute(methodDecl)) {
                                    for (Stmt *stmt : messageExpr->arguments()) {
                                        ObjCMessageExpr *callClassExpr = dyn_cast_or_null<ObjCMessageExpr>(stmt);
                                        if (callClassExpr && callClassExpr->getSelector().getAsString() == "class") {
                                            string leftType = removePtrString(getPropertyType(propertyName));
                                            string rightType = removePtrString(callClassExpr->getClassReceiver().getAsString());
                                            DiagnosticsEngine &diag = Instance.getDiagnostics();
                                            if (leftType.find(rightType) == std::string::npos) {
                                                FixItHint fixItHint = FixItHint::CreateReplacement(callClassExpr->getReceiverRange(), leftType);
                                                diag.Report(binaryOperator->getLocStart(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "类型不一致：左边：%0 右边：%1")) << leftType << rightType << fixItHint;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        string getPropertyType(const string propertyName)
        {
            vector<ObjCPropertyDecl *>::iterator iter;
            for (iter = propertyDeclVector.begin(); iter != propertyDeclVector.end(); iter++) {
                ObjCPropertyDecl *decl = *iter;
                if (decl->getName() == propertyName) {
                    return decl->getType().getAsString();
                }
            }
            return NULL;
        }
        
        string removePtrString(const string typeString)
        {
            size_t lastindex = typeString.find_last_of("*");
            return typeString.substr(0, lastindex);
        }

        bool checkIfHasAttribute(ObjCMethodDecl *methodDecl)
        {
            for (Attr *attr : methodDecl->attrs()) {
                if (!strcmp(attr->getSpelling(), "objc_same_type")) {
                    return true;
                }
            }
            return false;
        }
        
        bool isUserSourceDecl(const Decl *decl)
        {
            string filename = Instance.getSourceManager().getFilename(decl->getSourceRange().getBegin()).str();
            return isUserSourceWithFilename(filename);
        }
        
        bool isUserSourceStmt(const Stmt *stmt)
        {
            string filename = Instance.getSourceManager().getFilename(stmt->getSourceRange().getBegin()).str();
            return isUserSourceWithFilename(filename);
        }
        
        bool isUserSourceWithFilename(const string filename)
        {
            if (filename.empty())
                return false;
            
            //非XCode中的源码都认为是用户源码
            if(filename.find("/Applications/Xcode.app/") == 0)
                return false;
            
            return true;
        }
    };
    
 
    class SameTypePluginASTConsumer: public ASTConsumer
    {
    public:
        SameTypePluginASTConsumer(CompilerInstance &Instance) : handlerForSameType(Instance)
        {
            matcher.addMatcher(objcPropertyDecl().bind("objcPropertyDecl"), &handlerForSameType);
            
            matcher.addMatcher(binaryOperator(hasOperatorName("=")).bind("binaryOperator"), &handlerForSameType);
        }
        
    private:
        MatchFinder matcher;
        SameTypeHandler handlerForSameType;

        void HandleTranslationUnit(ASTContext &context)
        {
            handlerForSameType.setContext(context);
            matcher.matchAST(context);
        }
        
    };

    class SameTypePluginASTAction: public PluginASTAction
    {
    public:
        unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &Compiler, StringRef InFile) 
        {
            return unique_ptr<SameTypePluginASTConsumer>(new SameTypePluginASTConsumer(Compiler));
        }

        bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args)
        {
            return true;
        }
    };
}

static clang::FrontendPluginRegistry::Add<SameTypePlugin::SameTypePluginASTAction>
X("SameTypePlugin", "check parameter and return type");

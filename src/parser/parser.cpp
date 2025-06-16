// standard
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>

// common
#include <common/action-queue.hpp>
#include <common/common.hpp>

//algorithms
#include <algorithms/traversal.hpp>

// local
#include "parser.hpp"

using namespace parser;


namespace {

    template<typename TAction, typename... Args>
    std::shared_ptr<common::IAction> makeAction(Args... args) {
        return std::make_shared<TAction>(args...);
    }
} // anon namespace

void LexemeParser::reset() {
    shared = SharedState();
    shared.graph = std::make_shared<common::TraversalGraph>(); // <- Изменено
    //shared.graph = std::make_shared<common::Graph>();
}

void LexemeParser::entry() {}
void LexemeParser::exit() {}

void Idle::react(InputGraphType const &event) {
    if (event.graphType == "graph") {
        // Do nothing
    } else if (event.graphType == "digraph") {
        shared.flags |= common::opt::drc;
    } else {
        throw std::runtime_error("Invalid graph type, which is " + event.graphType);
    }
    transit<GraphType>();
}
    
void GraphType::react(InputOpenCurlyBracket const &) {
    transit<OpenCurlyBracket>();  
}

void OpenCurlyBracket::react(InputNodeId const &event) {
    LexemeParser::shared.fromNodeId = event.NodeID;
    transit<FromNodeID>();
}

void OpenCurlyBracket::react(InputCloseCurlyBracket const &) {
    transit<Idle>();
}

void FromNodeID::react(InputCloseCurlyBracket const &event) {
    shared.actionQueue.query(makeAction<common::PushNodeAction>(
        &common::Graph::pushNode, 
        shared.graph.get(), 
        LexemeParser::shared.fromNodeId));
    transit<Idle>();
}

void FromNodeID::react(InputOpenSquareBracket const &) {
    shared.actionQueue.query(makeAction<common::PushNodeAction>(
        &common::Graph::pushNode, 
        shared.graph.get(), 
        LexemeParser::shared.fromNodeId));
    LexemeParser::shared.expectedValue = "label";
    transit<OpenSquareBracket>();
}

void FromNodeID::react(InputEdge const &event) {
    shared.actionQueue.query(makeAction<common::PushNodeAction>(
        &common::Graph::pushNode, 
        shared.graph.get(), 
        LexemeParser::shared.fromNodeId));
    transit<Edge>();
}

void FromNodeID::react(InputNodeId const &event) {
    shared.actionQueue.query(makeAction<common::PushNodeAction>(
        &common::Graph::pushNode, 
        shared.graph.get(), 
        event.NodeID));
    transit<FromNodeID>();
    LexemeParser::shared.fromNodeId = event.NodeID;
}

void OpenSquareBracket::react(InputLabel const &event) {
    transit<Label>();
}

void Edge::react(InputNodeId const &event) {
    LexemeParser::shared.toNodeId = event.NodeID;
    transit<ToNodeID>();
}

void ToNodeID::react(InputCloseCurlyBracket const &event) {
    shared.actionQueue.query(makeAction<common::PushNodeAction>(
        &common::Graph::pushNode, 
        shared.graph.get(), 
        LexemeParser::shared.toNodeId));
    if (!(shared.flags & common::opt::wgh)) {
        shared.actionQueue.query(makeAction<common::PushEdgeAction>(
            &common::Graph::pushEdge, 
            shared.graph.get(), 
            LexemeParser::shared.fromNodeId,
            common::Connection(LexemeParser::shared.toNodeId, 1)));
    } else {
        shared.actionQueue.query(makeAction<common::PushEdgeAction>(
            &common::Graph::pushEdge, 
            shared.graph.get(), 
            LexemeParser::shared.fromNodeId,
            common::Connection(LexemeParser::shared.toNodeId)));
    }
    transit<Idle>();
}

void ToNodeID::react(InputOpenSquareBracket const &) {
    LexemeParser::shared.expectedValue = "weight";
    transit<OpenSquareBracket>();
}

void ToNodeID::react(InputNodeId const &event) {
    shared.actionQueue.query(makeAction<common::PushNodeAction>(
        &common::Graph::pushNode, 
        shared.graph.get(), 
        LexemeParser::shared.toNodeId));
    shared.actionQueue.query(makeAction<common::PushEdgeAction>(
        &common::Graph::pushEdge, 
        shared.graph.get(), 
        LexemeParser::shared.fromNodeId,
        common::Connection(LexemeParser::shared.toNodeId)));
    
    LexemeParser::shared.fromNodeId = event.NodeID;

    transit<FromNodeID>();
}

void Label::react(InputEqual const &) {
    transit<Equal>();
}

void Equal::react(InputStringValue const &event) {
    if (LexemeParser::shared.expectedValue == "label") {
        LexemeParser::shared.label = event.label;
        transit<Value>();
    } else {
        throw_invalid_input("Expected label, found: " + LexemeParser::shared.expectedValue);
    }
}

void Equal::react(InputIntValue const &event) {
    if (LexemeParser::shared.expectedValue == "weight") {
        LexemeParser::shared.weight = event.weight;
        transit<Value>();
    } else {
        throw_invalid_input("Expected weight, found: " + LexemeParser::shared.expectedValue);
    }
}

void Value::react(InputCloseSquareBracket const &event) {
    if (LexemeParser::shared.expectedValue == "weight") {
        shared.flags |= common::opt::wgh;
        shared.actionQueue.query(makeAction<common::PushNodeAction>(
            &common::Graph::pushNode, 
            shared.graph.get(), 
            LexemeParser::shared.toNodeId));
        shared.actionQueue.query(makeAction<common::PushEdgeAction>(
            &common::Graph::pushEdge, 
            shared.graph.get(), 
            LexemeParser::shared.fromNodeId,
            common::Connection(LexemeParser::shared.toNodeId, LexemeParser::shared.weight)));
    } else if (LexemeParser::shared.expectedValue == "label") {
        shared.backoffQueue.query(makeAction<common::SetLabelAction>(
            &common::Graph::setLabel, 
            shared.graph.get(),
            LexemeParser::shared.fromNodeId,
            LexemeParser::shared.label));
    } 
    transit<OpenCurlyBracket>();
}

// Set base state
FSM_INITIAL_STATE(LexemeParser, Idle)

std::shared_ptr<common::TraversalGraph> parser::parse(std::vector<common::Lexeme>& input) { // <- Изменён тип
    LexemeParser::reset();
    LexemeParser::start();
    for (const auto& lexeme : input) {
        switch (lexeme.type) {
            case common::LexemeType::GRAPH_START_LABEL:
                LexemeParser::dispatch(InputGraphType{.graphType = std::any_cast<std::string>(lexeme.value)});
                break;

            case common::LexemeType::OPEN_CURLY_BRACKET:
                LexemeParser::dispatch(InputOpenCurlyBracket());
                break;

            case common::LexemeType::CLOSED_CURLY_BRACKET:
                LexemeParser::dispatch(InputCloseCurlyBracket());
                break;

            case common::LexemeType::NODE_ID:
                LexemeParser::dispatch(InputNodeId{.NodeID = std::any_cast<std::string>(lexeme.value)});
                break;

            case common::LexemeType::POINTED_ARROW:
                if (LexemeParser::shared.flags & common::opt::drc) {
                    LexemeParser::dispatch(InputEdge());
                } else {
                    throw_invalid_input("Graph is not directional!");
                }
                break;

            case common::LexemeType::FLAT_ARROW:
                if (!(LexemeParser::shared.flags & common::opt::drc)) {
                    LexemeParser::dispatch(InputEdge());
                } else {
                    throw_invalid_input("Graph is directional");
                }
                break;

            case common::LexemeType::OPEN_SQUARE_BRACKET:
                LexemeParser::dispatch(InputOpenSquareBracket());
                break;

            case common::LexemeType::CLOSED_SQUARE_BRACKET:
                LexemeParser::dispatch(InputCloseSquareBracket());
                break;

            case common::LexemeType::LABEL_ATTRIBUTE:
                LexemeParser::dispatch(InputLabel());
                break;

            case common::LexemeType::EQUALS_SIGN:
                LexemeParser::dispatch(InputEqual());
                break;

            case common::LexemeType::ATTRIBUTE_STRING_VALUE:
                LexemeParser::dispatch(InputStringValue{.label = std::any_cast<std::string>(lexeme.value)});
                break;

            case common::LexemeType::ATTRIBUTE_INT_VALUE:
                LexemeParser::dispatch(InputIntValue{.weight = std::any_cast<int>(lexeme.value)});
                break;

            default:
                throw_invalid_input("...");
        };
    }

    LexemeParser::shared.graph->init(LexemeParser::shared.flags);
    LexemeParser::shared.actionQueue.dumpAllActions();
    LexemeParser::shared.backoffQueue.dumpAllActions();
    return std::dynamic_pointer_cast<common::TraversalGraph>(LexemeParser::shared.graph);

}

// // standard
// #include <iostream>
// #include <memory>
// #include <ostream>
// #include <stdexcept>

// // common
// #include <common/action-queue.hpp>
// #include <common/common.hpp>

// //algorithms
// #include <algorithms/traversal.hpp>

// // local
// #include "parser.hpp"

// using namespace parser;


// namespace {

//     template<typename TAction, typename... Args>
//     std::shared_ptr<common::IAction> makeAction(Args... args) {
//         return std::make_shared<TAction>(args...);
//     }

 
// } // anon namespace

// void LexemeParser::reset() {
//     shared = SharedState();
//     shared.graph = std::make_shared<common::TraversalGraph>(); // <- Изменено
//     //shared.graph = std::make_shared<common::Graph>();
// }

// void LexemeParser::entry() {}
// void LexemeParser::exit() {}

// void Idle::react(InputGraphType const &event) {
//     if (event.graphType == "graph") {
//         // Do nothing
//     } else if (event.graphType == "digraph") {
//         shared.flags |= common::opt::drc;
//     } else {
//         throw std::runtime_error("Invalid graph type, which is " + event.graphType);
//     }
//     transit<GraphType>();
// }
    
// void GraphType::react(InputOpenCurlyBracket const &) {
//     transit<OpenCurlyBracket>();  
// }

// void OpenCurlyBracket::react(InputNodeId const &event) {
//     LexemeParser::shared.fromNodeId = event.NodeID;
//     transit<FromNodeID>();
// }

// void OpenCurlyBracket::react(InputCloseCurlyBracket const &) {
//     transit<Idle>();
// }

// void FromNodeID::react(InputCloseCurlyBracket const &event) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode, 
//         shared.graph.get(), 
//         LexemeParser::shared.fromNodeId));
//     transit<Idle>();
// }

// void FromNodeID::react(InputOpenSquareBracket const &) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode, 
//         shared.graph.get(), 
//         LexemeParser::shared.fromNodeId));
//     LexemeParser::shared.expectedValue = "label";
//     transit<OpenSquareBracket>();
// }

// void FromNodeID::react(InputEdge const &event) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode, 
//         shared.graph.get(), 
//         LexemeParser::shared.fromNodeId));
//     transit<Edge>();
// }

// void FromNodeID::react(InputNodeId const &event) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode, 
//         shared.graph.get(), 
//         event.NodeID));
//     transit<FromNodeID>();
//     LexemeParser::shared.fromNodeId = event.NodeID;
// }

// void OpenSquareBracket::react(InputAttrKey const &event) {
//     LexemeParser::shared.currentAttr = event.key;
//     transit<Equal>();
// }

// void OpenSquareBracket::react(InputLabel const &) {
//     LexemeParser::shared.currentAttr = "label";
//     transit<Equal>(); // ADDED
// }

// void OpenSquareBracket::react(InputWeight const &) {
//     LexemeParser::shared.currentAttr = "weight";
//     transit<Equal>(); // ADDED
// }


// void Edge::react(InputNodeId const &event) {
//     LexemeParser::shared.toNodeId = event.NodeID;
//     transit<ToNodeID>();
// }

// void ToNodeID::react(InputCloseCurlyBracket const &event) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode, 
//         shared.graph.get(), 
//         LexemeParser::shared.toNodeId));
//     if (!(shared.flags & common::opt::wgh)) {
//         shared.actionQueue.query(makeAction<common::PushEdgeAction>(
//             &common::Graph::pushEdge, 
//             shared.graph.get(), 
//             LexemeParser::shared.fromNodeId,
//             common::Connection(LexemeParser::shared.toNodeId, 1)));
//     } else {
//         shared.actionQueue.query(makeAction<common::PushEdgeAction>(
//             &common::Graph::pushEdge, 
//             shared.graph.get(), 
//             LexemeParser::shared.fromNodeId,
//             common::Connection(LexemeParser::shared.toNodeId)));
//     }
//     transit<Idle>();
// }



// void ToNodeID::react(InputOpenSquareBracket const &) {
//     transit<Attributes>();
// }


// void ToNodeID::react(InputNodeId const &event) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode, 
//         shared.graph.get(), 
//         LexemeParser::shared.toNodeId));
//     shared.actionQueue.query(makeAction<common::PushEdgeAction>(
//         &common::Graph::pushEdge, 
//         shared.graph.get(), 
//         LexemeParser::shared.fromNodeId,
//         common::Connection(LexemeParser::shared.toNodeId)));
    
//     LexemeParser::shared.fromNodeId = event.NodeID;

//     transit<FromNodeID>();
// }

// void Label::react(InputEqual const &) {
//     transit<Equal>();
// }


// void Equal::react(InputEqual const &) {
//     transit<AttrValue>();
// }


// // void Equal::react(InputIntValue const &event) {
// //     if (LexemeParser::shared.currentAttr == "weight") {
// //         LexemeParser::shared.weight = event.weight;
// //         transit<Value>();
// //     } else {
// //         throw_invalid_input("Unexpected int attribute: " + LexemeParser::shared.currentAttr);
// //     }
// // }

// // void Equal::react(InputStringValue const &event) {
// //     if (LexemeParser::shared.currentAttr == "label") {
// //         LexemeParser::shared.label = event.label;
// //         transit<Value>();
// //     } else {
// //         throw_invalid_input("Unexpected string attribute: " + LexemeParser::shared.currentAttr);
// //     }
// // }





// //-----------adds

// // parser.cpp

// void Attributes::react(InputAttrKey const &event) {
//     LexemeParser::shared.currentAttr = event.key;
//     transit<Equal>();
// }

// void Attributes::react(InputEqual const &) {
//     transit<AttrValue>();
// }

// void Attributes::react(InputCloseSquareBracket const &) {
//     shared.flags |= common::opt::wgh;

//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode,
//         shared.graph.get(),
//         LexemeParser::shared.toNodeId));
    
//     shared.actionQueue.query(makeAction<common::PushEdgeAction>(
//         &common::Graph::pushEdge,
//         shared.graph.get(),
//         LexemeParser::shared.fromNodeId,
//         common::Connection(LexemeParser::shared.toNodeId, LexemeParser::shared.weight)));

//     if (!LexemeParser::shared.label.empty()) {
//         shared.backoffQueue.query(makeAction<common::SetLabelAction>(
//             &common::Graph::setLabel,
//             shared.graph.get(),
//             LexemeParser::shared.fromNodeId,
//             LexemeParser::shared.label));
//     }

//     // Сбросим для следующего ребра
//     LexemeParser::shared.label.clear();
//     LexemeParser::shared.weight = 1;
//     LexemeParser::shared.currentAttr.clear();

//     transit<OpenCurlyBracket>();
// }

// void AttrValue::react(InputStringValue const &event) {
//     if (LexemeParser::shared.currentAttr == "label") {
//         LexemeParser::shared.label = event.label;
//     } else {
//         throw_invalid_input("Unexpected string attribute value for: " + LexemeParser::shared.currentAttr);
//     }
//     transit<Attributes>();
// }

// void AttrValue::react(InputIntValue const &event) {
//     if (LexemeParser::shared.currentAttr == "weight") {
//         LexemeParser::shared.weight = event.weight;
//     } else {
//         throw_invalid_input("Unexpected int attribute value for: " + LexemeParser::shared.currentAttr);
//     }
//     transit<Attributes>();
// }




// void Value::react(InputCloseSquareBracket const &event) {
//     // Применяем label и weight, если они есть
//     if (LexemeParser::shared.currentAttr == "weight") {
//         shared.flags |= common::opt::wgh;
//         shared.actionQueue.query(makeAction<common::PushNodeAction>(
//             &common::Graph::pushNode,
//             shared.graph.get(),
//             LexemeParser::shared.toNodeId));
//         shared.actionQueue.query(makeAction<common::PushEdgeAction>(
//             &common::Graph::pushEdge,
//             shared.graph.get(),
//             LexemeParser::shared.fromNodeId,
//             common::Connection(LexemeParser::shared.toNodeId, LexemeParser::shared.weight)));
//     }
//     if (LexemeParser::shared.currentAttr == "label") {
//         shared.backoffQueue.query(makeAction<common::SetLabelAction>(
//             &common::Graph::setLabel,
//             shared.graph.get(),
//             LexemeParser::shared.fromNodeId,
//             LexemeParser::shared.label));
//     }
//     // Сбрасываем текущие значения для следующего атрибута
//     LexemeParser::shared.label.clear();
//     LexemeParser::shared.weight = 1; // По умолчанию
//     LexemeParser::shared.currentAttr.clear();

//     transit<OpenCurlyBracket>();
// }


// // Set base state
// FSM_INITIAL_STATE(LexemeParser, Idle)

// std::shared_ptr<common::TraversalGraph> parse(std::vector<common::Lexeme>& input) { // <- Изменён тип
//     LexemeParser::reset();
//     LexemeParser::start();
//     for (const auto& lexeme : input) {
//         switch (lexeme.type) {
//             case common::LexemeType::GRAPH_START_LABEL:
//                 LexemeParser::dispatch(InputGraphType{.graphType = std::any_cast<std::string>(lexeme.value)});
//                 break;

//             case common::LexemeType::OPEN_CURLY_BRACKET:
//                 LexemeParser::dispatch(InputOpenCurlyBracket());
//                 break;

//             case common::LexemeType::CLOSED_CURLY_BRACKET:
//                 LexemeParser::dispatch(InputCloseCurlyBracket());
//                 break;

//             case common::LexemeType::NODE_ID:
//                 LexemeParser::dispatch(InputNodeId{.NodeID = std::any_cast<std::string>(lexeme.value)});
//                 break;

//             case common::LexemeType::POINTED_ARROW:
//                 if (LexemeParser::shared.flags & common::opt::drc) {
//                     LexemeParser::dispatch(InputEdge());
//                 } else {
//                     throw_invalid_input("Graph is not directional!");
//                 }
//                 break;

//             case common::LexemeType::FLAT_ARROW:
//                 if (!(LexemeParser::shared.flags & common::opt::drc)) {
//                     LexemeParser::dispatch(InputEdge());
//                 } else {
//                     throw_invalid_input("Graph is directional");
//                 }
//                 break;

//             case common::LexemeType::OPEN_SQUARE_BRACKET:
//                 LexemeParser::dispatch(InputOpenSquareBracket());
//                 break;

//             case common::LexemeType::CLOSED_SQUARE_BRACKET:
//                 LexemeParser::dispatch(InputCloseSquareBracket());
//                 break;

//             case common::LexemeType::LABEL_ATTRIBUTE:
//                 LexemeParser::dispatch(InputLabel());
//                 break;

//             case common::LexemeType::EQUALS_SIGN:
//                 LexemeParser::dispatch(InputEqual());
//                 break;

//             case common::LexemeType::ATTRIBUTE_STRING_VALUE:
//                 LexemeParser::dispatch(InputStringValue{.label = std::any_cast<std::string>(lexeme.value)});
//                 break;

//             case common::LexemeType::ATTRIBUTE_INT_VALUE:
//                 LexemeParser::dispatch(InputIntValue{.weight = std::any_cast<int>(lexeme.value)});
//                 break;
//             case common::LexemeType::ATTRIBUTE_KEY:
//             LexemeParser::dispatch(InputAttrKey{.key = std::any_cast<std::string>(lexeme.value)}); // OK
//                 break;
            

//             default:
//                 throw_invalid_input("...");
//         };
//     }

//     LexemeParser::shared.graph->init(LexemeParser::shared.flags);
//     LexemeParser::shared.actionQueue.dumpAllActions();
//     LexemeParser::shared.backoffQueue.dumpAllActions();
//     return std::dynamic_pointer_cast<common::TraversalGraph>(LexemeParser::shared.graph);

// }








// // standard
// #include <iostream>
// #include <memory>
// #include <ostream>
// #include <stdexcept>

// // common
// #include <common/action-queue.hpp>
// #include <common/common.hpp>

// // algorithms
// #include <algorithms/traversal.hpp>

// // local
// #include "parser.hpp"

// using namespace parser;

// namespace {
//     template<typename TAction, typename... Args>
//     std::shared_ptr<common::IAction> makeAction(Args... args) {
//         return std::make_shared<TAction>(args...);
//     }
// }

// void LexemeParser::reset() {
//     shared = SharedState();
//     shared.graph = std::make_shared<common::TraversalGraph>();
// }

// void LexemeParser::entry() {}
// void LexemeParser::exit() {}

// void Idle::react(InputGraphType const &event) {
//     if (event.graphType == "graph") {
//         // do nothing
//     } else if (event.graphType == "digraph") {
//         shared.flags |= common::opt::drc;
//     } else {
//         throw std::runtime_error("Invalid graph type: " + event.graphType);
//     }
//     transit<GraphType>();
// }

// void GraphType::react(InputOpenCurlyBracket const &) {
//     transit<OpenCurlyBracket>();
// }

// void OpenCurlyBracket::react(InputNodeId const &event) {
//     LexemeParser::shared.fromNodeId = event.NodeID;
//     transit<FromNodeID>();
// }

// void OpenCurlyBracket::react(InputCloseCurlyBracket const &) {
//     transit<Idle>();
// }

// void FromNodeID::react(InputCloseCurlyBracket const &) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode,
//         shared.graph.get(),
//         LexemeParser::shared.fromNodeId));
//     transit<Idle>();
// }

// void FromNodeID::react(InputOpenSquareBracket const &) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode,
//         shared.graph.get(),
//         LexemeParser::shared.fromNodeId));
//     LexemeParser::shared.expectedValue = "label";
//     transit<OpenSquareBracket>();
// }

// void FromNodeID::react(InputEdge const &) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode,
//         shared.graph.get(),
//         LexemeParser::shared.fromNodeId));
//     transit<Edge>();
// }

// void FromNodeID::react(InputNodeId const &event) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode,
//         shared.graph.get(),
//         event.NodeID));
//     LexemeParser::shared.fromNodeId = event.NodeID;
//     transit<FromNodeID>();
// }

// void OpenSquareBracket::react(InputAttrKey const &event) {
//     LexemeParser::shared.currentAttr = event.key;
//     transit<Equal>();
// }

// void OpenSquareBracket::react(InputLabel const &) {
//     LexemeParser::shared.currentAttr = "label";
//     transit<Equal>();
// }

// void OpenSquareBracket::react(InputWeight const &) {
//     LexemeParser::shared.currentAttr = "weight";
//     transit<Equal>();
// }

// void Edge::react(InputNodeId const &event) {
//     LexemeParser::shared.toNodeId = event.NodeID;
//     transit<ToNodeID>();
// }

// void ToNodeID::react(InputCloseCurlyBracket const &) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode,
//         shared.graph.get(),
//         LexemeParser::shared.toNodeId));

//     if (!(shared.flags & common::opt::wgh)) {
//         shared.actionQueue.query(makeAction<common::PushEdgeAction>(
//             &common::Graph::pushEdge,
//             shared.graph.get(),
//             LexemeParser::shared.fromNodeId,
//             common::Connection(LexemeParser::shared.toNodeId, 1)));
//     } else {
//         shared.actionQueue.query(makeAction<common::PushEdgeAction>(
//             &common::Graph::pushEdge,
//             shared.graph.get(),
//             LexemeParser::shared.fromNodeId,
//             common::Connection(LexemeParser::shared.toNodeId, LexemeParser::shared.weight)));
//     }

//     if (!LexemeParser::shared.label.empty()) {
//         shared.backoffQueue.query(makeAction<common::SetLabelAction>(
//             &common::Graph::setLabel,
//             shared.graph.get(),
//             LexemeParser::shared.fromNodeId,
//             LexemeParser::shared.label));
//     }

//     LexemeParser::shared.label.clear();
//     LexemeParser::shared.weight = 1;
//     LexemeParser::shared.currentAttr.clear();

//     transit<Idle>();
// }

// void ToNodeID::react(InputOpenSquareBracket const &) {
//     transit<Attributes>();
// }

// void ToNodeID::react(InputNodeId const &event) {
//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode,
//         shared.graph.get(),
//         LexemeParser::shared.toNodeId));
//     shared.actionQueue.query(makeAction<common::PushEdgeAction>(
//         &common::Graph::pushEdge,
//         shared.graph.get(),
//         LexemeParser::shared.fromNodeId,
//         common::Connection(LexemeParser::shared.toNodeId)));

//     LexemeParser::shared.fromNodeId = event.NodeID;
//     transit<FromNodeID>();
// }

// void Label::react(InputEqual const &) {
//     transit<Equal>();
// }

// void Equal::react(InputEqual const &) {
//     transit<AttrValue>();
// }

// void AttrValue::react(InputStringValue const &event) {
//     if (LexemeParser::shared.currentAttr == "label") {
//         LexemeParser::shared.label = event.label;
//     } else {
//         throw_invalid_input("Unexpected string attribute: " + LexemeParser::shared.currentAttr);
//     }
//     transit<Attributes>();
// }

// void AttrValue::react(InputIntValue const &event) {
//     if (LexemeParser::shared.currentAttr == "weight") {
//         LexemeParser::shared.weight = event.weight;
//     } else {
//         throw_invalid_input("Unexpected int attribute: " + LexemeParser::shared.currentAttr);
//     }
//     transit<Attributes>();
// }

// void Attributes::react(InputAttrKey const &event) {
//     LexemeParser::shared.currentAttr = event.key;
//     transit<Equal>();
// }

// void Attributes::react(InputEqual const &) {
//     transit<AttrValue>();
// }

// void Attributes::react(InputCloseSquareBracket const &) {
//     shared.flags |= common::opt::wgh;

//     shared.actionQueue.query(makeAction<common::PushNodeAction>(
//         &common::Graph::pushNode,
//         shared.graph.get(),
//         LexemeParser::shared.toNodeId));

//     shared.actionQueue.query(makeAction<common::PushEdgeAction>(
//         &common::Graph::pushEdge,
//         shared.graph.get(),
//         LexemeParser::shared.fromNodeId,
//         common::Connection(LexemeParser::shared.toNodeId, LexemeParser::shared.weight)));

//     if (!LexemeParser::shared.label.empty()) {
//         shared.backoffQueue.query(makeAction<common::SetLabelAction>(
//             &common::Graph::setLabel,
//             shared.graph.get(),
//             LexemeParser::shared.fromNodeId,
//             LexemeParser::shared.label));
//     }

//     LexemeParser::shared.label.clear();
//     LexemeParser::shared.weight = 1;
//     LexemeParser::shared.currentAttr.clear();

//     transit<OpenCurlyBracket>();
// }

// FSM_INITIAL_STATE(LexemeParser, Idle)

// std::shared_ptr<common::TraversalGraph> parse(std::vector<common::Lexeme>& input) {
//     LexemeParser::reset();
//     LexemeParser::start();

//     for (const auto& lexeme : input) {
//         switch (lexeme.type) {
//             case common::LexemeType::GRAPH_START_LABEL:
//                 LexemeParser::dispatch(InputGraphType{.graphType = std::any_cast<std::string>(lexeme.value)});
//                 break;

//             case common::LexemeType::OPEN_CURLY_BRACKET:
//                 LexemeParser::dispatch(InputOpenCurlyBracket());
//                 break;

//             case common::LexemeType::CLOSED_CURLY_BRACKET:
//                 LexemeParser::dispatch(InputCloseCurlyBracket());
//                 break;

//             case common::LexemeType::NODE_ID:
//                 LexemeParser::dispatch(InputNodeId{.NodeID = std::any_cast<std::string>(lexeme.value)});
//                 break;

//             case common::LexemeType::POINTED_ARROW:
//                 if (LexemeParser::shared.flags & common::opt::drc) {
//                     LexemeParser::dispatch(InputEdge());
//                 } else {
//                     throw_invalid_input("Graph is not directional!");
//                 }
//                 break;

//             case common::LexemeType::FLAT_ARROW:
//                 if (!(LexemeParser::shared.flags & common::opt::drc)) {
//                     LexemeParser::dispatch(InputEdge());
//                 } else {
//                     throw_invalid_input("Graph is directional!");
//                 }
//                 break;

//             case common::LexemeType::OPEN_SQUARE_BRACKET:
//                 LexemeParser::dispatch(InputOpenSquareBracket());
//                 break;

//             case common::LexemeType::CLOSED_SQUARE_BRACKET:
//                 LexemeParser::dispatch(InputCloseSquareBracket());
//                 break;

//             case common::LexemeType::LABEL_ATTRIBUTE:
//                 LexemeParser::dispatch(InputLabel());
//                 break;

//             case common::LexemeType::WEIGHT_ATTRIBUTE:
//                 LexemeParser::dispatch(InputWeight());
//                 break;

//             case common::LexemeType::EQUALS_SIGN:
//                 LexemeParser::dispatch(InputEqual());
//                 break;

//             case common::LexemeType::ATTRIBUTE_STRING_VALUE:
//                 LexemeParser::dispatch(InputStringValue{.label = std::any_cast<std::string>(lexeme.value)});
//                 break;

//             case common::LexemeType::ATTRIBUTE_INT_VALUE:
//                 LexemeParser::dispatch(InputIntValue{.weight = std::any_cast<int>(lexeme.value)});
//                 break;

//             case common::LexemeType::ATTRIBUTE_KEY:
//                 LexemeParser::dispatch(InputAttrKey{.key = std::any_cast<std::string>(lexeme.value)});
//                 break;

//             default:
//                 throw_invalid_input("Unknown lexeme type.");
//         }
//     }

//     LexemeParser::shared.graph->init(LexemeParser::shared.flags);
//     LexemeParser::shared.actionQueue.dumpAllActions();
//     LexemeParser::shared.backoffQueue.dumpAllActions();
//     return std::dynamic_pointer_cast<common::TraversalGraph>(LexemeParser::shared.graph);
// }

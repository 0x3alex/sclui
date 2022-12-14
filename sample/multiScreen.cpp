#include "sclui.hpp"

using namespace sclui;

//needs to be pointer
Screen *subScreen1 = nullptr;
Screen *subScreen2 = nullptr;

//manual free method to free subscreens
void myQuit() {
    delete subScreen1;
    delete subScreen2;

    //call built in free method to do the rest
    doQuit();
}

void switchScreen() {
    Button b3 = Button("Quit",1,1,COLOR_WHITE,COLOR_RED);
    CheckBox c1 = CheckBox("Test",1,4,COLOR_WHITE,COLOR_BLUE,true);

    b3.onButtonPress = &doQuit;
    Screen newScreen = Screen("hallo",20,10,0,0);
    newScreen.addItem(&b3);
    newScreen.addItem(&c1);
    newScreen.draw();
}

int main(void) {
    initSclui();
    Screen mainScreen = Screen("",getMaxX(),getMaxY(),0,0);
    
    subScreen1 = new Screen("S1",20,10,0,0);
    Text text1 = Text("Hallo\nwelt",1,1,COLOR_BLANK);
    Button b1 = Button("quit",2,4,COLOR_WHITE,COLOR_RED);
    b1.onButtonPress = &doQuit;
    subScreen1->addItem(&text1);
    subScreen1->addItem(&b1);

    

    subScreen2 = new Screen("S2", 30, 15 , 12, 12);
    Button b4 = Button("next",2,4,COLOR_BLUE,COLOR_RED);
    TextBox text = TextBox("textbox",2,6,COLOR_WHITE,COLOR_GREEN,10, &TextBoxFilterIsText,'|');

    subScreen2->addItem(&b4);
    //example for get item by name
    ((Button*)subScreen2->getItemByName("next"))->onButtonPress = &switchScreen;
    subScreen2->addItem(&text);
    mainScreen.addSubScreen(subScreen1);
    mainScreen.addSubScreen(subScreen2);

    mainScreen.draw();
    return 0;
}
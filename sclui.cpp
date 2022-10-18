#include "sclui.h"
#include <cstring>
#include <curses.h>

static int cIndex = 2;

void initSclui() {
    WINDOW *w = initscr();
    assert(w != NULL);
    raw();
    noecho();
    start_color();
    keypad(stdscr,TRUE);
    clear();
}

bool isText(int c) {
    return isalpha(c) || c == ' ';
}

bool isNumber(int c) {
    return isdigit(c);
}

BasicItem::types BasicItem::getType() {
    return type;
}

std::string BasicItem::getName() {
    return name;
}
int BasicItem::getX() {
    return x;
}
int BasicItem::getY() {
    return y;
}

void BasicItem::setX(int pX) {
    x = pX;
}
void BasicItem::setY(int pY) {
    y = pY;
}

bool BasicItem::isInteractable() {
    return interactable;
}
void BasicItem::setInteractable(bool v) {
    interactable = v;
}
void BasicItem::moveTo() {
    move(y,x);
}

void BasicItem::setColor(bool b) {
    attroff(COLOR_PAIR(itemIndex));
    if(color == COLOR_BLANK) return;
    init_pair(itemIndex,COLOR_BLACK, b ? colorFocus : color);
    attron(COLOR_PAIR(itemIndex));
}


Button::Button(std::string pName, int pX, int pY, int pColor, int pColorFocus, void(*pActionEvent)()) {
    name = pName;
    type = BUTTON;
    color = pColor;
    colorFocus = pColorFocus;
    itemIndex = cIndex++;
    interactable = true;
    actionEvent = pActionEvent;
    x = pX;
    y = pY;
}

void Button::draw(bool v) {
    setColor(v);
    curs_set(0);
    this->moveTo();
    printw("[ %s ]" , name.c_str()); 
    this->moveTo();
}

void Button::getValue() {
    return;
}

CheckBox::CheckBox(std::string pName,int pX, int pY,int pColor, int pColorFocus, bool defaultValue) {
    name = pName;
    x = pX;
    y = pY;
    color = pColor;
    colorFocus = pColorFocus;
    value = defaultValue;
    interactable = true;
    type = CHECKBOX;
    itemIndex = cIndex++;
}

void CheckBox::draw(bool v) {
    setColor(v);
    curs_set(0);
    this->moveTo();
    printw("[ %s (%c) ]", name.c_str(), value == true ? 'X' : ' ');
    this->moveTo();
}

bool CheckBox::getValue() {
    return value;
}

void CheckBox::setValue(bool v) {
    value = v;
}

Text::Text(std::string pName,int pX, int pY,int pColor) {
    name = pName;
    x = pX;
    y = pY;
    color = pColor;
    interactable = false;
    type = BASIC;
    itemIndex = cIndex++;

}
void Text::draw(bool v) {
    setColor(v);
    curs_set(0);
    this->moveTo();
    printw("%s", name.c_str());
    this->moveTo();
}

std::string Text::getValue() {
    return name;
}

//screen
Screen::Screen(std::string pTitle,int pWidth, int pHeight, char pHFrame, char pVFrame) {
    width = pWidth;
    height = pHeight;
    hFrame = pHFrame;
    vFrame = pVFrame;
    title = pTitle;
}


void Screen::addItem(BasicItem *i) {
    items.push_back(i);
}

BasicItem *Screen::getFirstInteractableItem() {
    int index = 0;
    for(auto &i: items) {
        if(i->isInteractable()) {
            vecIndex = index;
            return i;
        }
        index++;
        
    }
    return NULL;
}

BasicItem *Screen::getItemByName(const char *name) {
    //auto generate type
    for(auto &i : items) {
        if(i->getName().compare(name)== 0) {
            return i;
        }
    }
    return nullptr;
}

TextBox::TextBox(std::string pName,int pX, int pY, int pMaxLength,int pColor, int pColorFocus, bool(*pFilter)(int)) {
    name = pName;
    x = pX;
    y = pY;
    maxLength = pMaxLength;
    itemIndex = cIndex++;
    interactable = true;
    type = TEXTBOX;
    color = pColor;
    colorFocus = pColorFocus;
    filter = pFilter;
}

int TextBox::getValueLength() {
    return value.length();
}

int TextBox::getMaxLength() {
    return maxLength;
}

void TextBox::draw(bool v) {
    setColor(v);
    curs_set(1);
    this->moveTo();
    printw("[ %s> " , name.c_str()); 
    for(int i = 0; i < maxLength; i++) printw(" ");
    printw("]");
    move(y,x+7);
    printw("%s",value.c_str());
}
std::string TextBox::getValue() {
    return value;
}
void TextBox::append(char c) {
    value.push_back(c);
}
void TextBox::pop() {
    value.pop_back();
}

void Screen::drawFrame() {
    move(0,0);
    init_pair(1,COLOR_BLACK,COLOR_WHITE);
    attron(COLOR_PAIR(1));
    for(int i = 0; i <= width; i++) {
        printw("%c",hFrame);
        if(i == 2) {
            addstr(title.c_str());
            i += title.length();
        }
    }
    move(height,0);
    for(int i = 0; i <= width; i++) {
        printw("%c",hFrame);
    }
    for(int i = 0; i <= height; i++) {
        move(i,0);
        printw("%c",vFrame);
    }
    for(int i = 0; i <= height; i++) {
        move(i,width);
        printw("%c",vFrame);
    }
    attroff(COLOR_PAIR(1));
}

bool Screen::selectNext(BasicItem *i) {
    if(!i->isInteractable()) return false;
    i->draw(true);
    return true;
}

void Screen::doMove(int mov) {
    if(mov == 0)  { //down
        for(int i = vecIndex+1; i < items.size(); i++) {
            if(selectNext(items.at(i))) {
                currentItem->draw(false);
                vecIndex = i;
                currentItem = items.at(i);
                currentItem->draw(true);
                return;
            }
        }
    }else{ //up
        for(int i = vecIndex-1; i >= 0; i--) {
            if(selectNext(items.at(i))) {
                currentItem->draw(false);
                vecIndex = i;
                currentItem = items.at(i);
                currentItem->draw(true);
                return;
            }
        } 
    }
    refresh();
}

void Screen::drawItems() {
    for(auto &i : items) {
        i->draw(false);
    }
    vecIndex = 0;
    currentItem = getFirstInteractableItem();
    
}

void Screen::run() {
    if(currentItem != NULL) {
        currentItem->draw(true);
    }
    CheckBox *checkBoxCast = nullptr;
    Button *buttonCast = nullptr;
    TextBox *textBoxCast = nullptr;
    int c;
    for(;;) {
        c = getch();
        switch(c) {
            case KEY_UP:
                doMove(1);
                break;
            case KEY_DOWN:
                doMove(0);
                break;
            case ' ':
                switch(currentItem->getType()) {
                    case BasicItem::CHECKBOX:
                        checkBoxCast = (CheckBox*) currentItem;
                        checkBoxCast->setValue((bool*)(checkBoxCast->getValue())?false:true);
                        break;
                    case BasicItem::BUTTON:
                        buttonCast = (Button*) currentItem;
                        (*(buttonCast->actionEvent))();
                        break;
                    case BasicItem::TEXTBOX:
                        goto textBoxHandler;
                        break;
                }
                currentItem->draw(true);
                break;
            default:
                textBoxHandler:
                    if(currentItem->getType() == BasicItem::TEXTBOX) {
                        textBoxCast = (TextBox*) currentItem;
                        if(c == KEY_BACKSPACE && textBoxCast->getValueLength() > 0) {
                            textBoxCast->pop();
                        }else{
                            if(textBoxCast->getValueLength() < textBoxCast->getMaxLength() && (*(textBoxCast->filter))(c)) {
                                textBoxCast->append(c);
                            }
                        }
                        currentItem->draw(true);
                    }
                break;
        }
        refresh();
    }
}

void Screen::draw() {
    drawFrame();
    drawItems();
    refresh();
    run();
}

void Screen::centerItem(Screen::axis pAxis, BasicItem *i) {
    TextBox *textBoxCast = nullptr;
    switch(pAxis) {
        case X:
            switch(i->getType()) {
                case BasicItem::TEXTBOX:
                    textBoxCast = (TextBox*) i;
                    i->setX((width / 2) - ((i->getName().length() /2) + ((textBoxCast->getMaxLength() + 4) / 2)));
                    break;
                case BasicItem::CHECKBOX:
                     i->setX((width / 2) - ((i->getName().length() /2) + 4));
                    break;
                case BasicItem::BUTTON:
                    i->setX((width / 2) - ((i->getName().length() /2) + 2));
                    break;
                default:
                     i->setX((width / 2) - (i->getName().length() /2));
                     break;
            }
           
            break;
        case Y:
            i->setY(height / 2);
            break;
        default:
            centerItem(X,i);
            centerItem(Y,i);
            break;
    }
}
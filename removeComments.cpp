#include<iostream>
#include <string>
using namespace std;
#define keepIndent

/*
* Function to check the line character by character and tracking the comments
*/
bool checkLine(string &line, bool &spanStart)
{
    int index = 0;
    int size = line.size();
	//to keep track of character printed apart from spaces
    bool containChar = false;
    
    //to keep the indentetion correct in case of multi- 
    //line comment and code in same line 
#ifdef keepIndent
    string spaces;
    while(index < size-1 && line[index] == ' ')
    {
        spaces += ' ';
        index++;
    }
#endif

    //check till size-1 last character need to be print separately
    while(index < size-1)
    {
        if(!spanStart && line[index] != '/')
        {
        #ifdef keepIndent
            if(!containChar)
                cout << spaces;
        #endif
            containChar = true;
            cout << line[index];
        }
        
        switch(line[index])
        {
            case '/':
            {
                if(!spanStart){
                    if(line[index + 1] == '/')
                        return containChar;
                    else if(line[index + 1] == '*'){
                        spanStart = true;
                        index++;
                    }
                }
                break;    
            }
            case '*':
            {
                if(spanStart && line[index+1] == '/'){
                    spanStart = false;
                    index++;
                }
                break;
            }
        }
        index++;
    }
	//print last remaining charater with indentention preserved 
	//if it is not just a space
    if(!spanStart && index < size && line[index] != ' ')
    {
    #ifdef keepIndent
        if(!containChar)
            cout << spaces;
    #endif
        containChar = true;
        cout<< line[index];
    }
    return containChar;
}

int main() 
{
    string line;
    //keep track of multiline commnets
	bool spanStart;
    bool result;
	//get lines one by one
    while (getline(cin, line)) 
    {
        result = checkLine(line, spanStart);
        //only add new line if any character apart from //space was printed for this line.
		if(result == true)
            cout << endl;
    }
    return 0;
}
#define CM_DEF(NAME, PARAM, BODY) \
    bool NAME PARAM BODY

CM_DEF(IsUser, (int a, float b), {
    return false;
})

#define CM(NAME) NAME

bool IsUserA(int a, float b) {return true;}

int main() {
    int a = 0;
    for(int i = 0; i < 4; i++) {
        a++;
    }
    CM(IsUser)(1, 2);
    auto f = CM(IsUser);
    f(2, 3.0);
    IsUserA(1, 2);    
    return 0;
}
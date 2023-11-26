#include <cstdio>
#include <cstdlib>

int main()
{
    while (true)
    {
        int health = 100;

        while (health > 0)
        {
            printf("Health: %d", health);
            getchar();
            health -= 1 + rand() % 5;
        }
    }

    return 0;
}
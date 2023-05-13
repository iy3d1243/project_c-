#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
using namespace sf;

const int num = 8; //direction des voitures
int points[num][2] = { 300, 610,
                      1270,430,
                      1380,2380,
                      1900,2460,
                      1970,1700,
                      2550,1680,
                      2560,3150,
                      500, 3300 };

struct Car
{
    float x, y, speed, angle; 
    int n;

    Car() { speed = 0; angle = 0; n = 0; /* proprietes initials de chaque  voiture */ }

    void move()
    {
        x += sin(angle) * speed;
        y -= cos(angle) * speed;
    }

    void direction()
    {
        float tx = points[n][0];// direction a suivre selon x 
        float ty = points[n][1];// direction a suivre selon y 
        float beta = angle - atan2(tx - x, -ty + y);// l'angle entre l'axe x et y 
        if (sin(beta) < 0) angle += 0.005 * speed;
        else angle -= 0.005 * speed;
        if ((x - tx) * (x - tx) + (y - ty) * (y - ty) < 25 * 25) n = (n + 1) % num;
    }
};


int main()
{
    RenderWindow app(VideoMode(900, 680), "rush");
    app.setFramerateLimit(30); // fps , peut defenir le vitesse de jeux 



    Texture t1, t2, t3;
    t1.loadFromFile("track.png");
    t2.loadFromFile("car.png");
    t1.setSmooth(true);
    t2.setSmooth(true);

    Sprite sBackground(t1), sCar(t2);
    sBackground.scale(2, 2);// dimension arriere plan 
    sCar.scale(0.12, 0.12); // dimension des voiture ( dimension original)  
    sCar.setOrigin(40, 60);//defenir le centre du voiture (l'axe de rotation ) 
    float R = 22;// domaine de colision 

    const int N = 5; // nbre participants (max 10 ) 
    Car car[N];
    for (int i = 0; i < N; i++)
    {
        car[i].x = 300 + i * 50; // position intial de x 
        car[i].y = 1700 + i * 80;// position innitial de y 
        car[i].speed = 7 + i; // vitesse des voitures, chaque voiture a son  vitesse 
    }

    float speed = 0, angle = 0; // pos initial
    float vitessemax = 12.0;
    float acc = 0.2, dec = 0.3;
    float turnSpeed = 0.1;


    Music gameMusic;
    gameMusic.openFromFile("song.ogg");
    gameMusic.setVolume(30);
    gameMusic.play();
    gameMusic.setLoop(true);



    SoundBuffer horn;
    if (!horn.loadFromFile("horn.ogg"))
    {
        exit(0);
    }
    Sound hornsound;
    hornsound.setBuffer(horn);
    hornsound.setVolume(10);


    SoundBuffer contact;
    if (!contact.loadFromFile("hit.ogg"))
    {
        exit(0);
    }
    Sound contactsound;
    contactsound.setBuffer(contact);
    contactsound.setVolume(60);


    while (app.isOpen())
    {
        Event e;
        while (app.pollEvent(e))
        {
            if (e.type == Event::Closed)
                app.close();
        }

        bool Up = 0, Right = 0, Down = 0, Left = 0;
        if (Keyboard::isKeyPressed(Keyboard::Up)) Up = 1;
        if (Keyboard::isKeyPressed(Keyboard::Right)) Right = 1;
        if (Keyboard::isKeyPressed(Keyboard::Down)) Down = 1;
        if (Keyboard::isKeyPressed(Keyboard::Left)) Left = 1;
        if (Keyboard::isKeyPressed(Keyboard::Escape)) exit(0);
        if (Keyboard::isKeyPressed(Keyboard::Space)) hornsound.play();


        //mouvement
        if (Up && speed < vitessemax)
            if (speed < 0)  speed += dec;
            else  speed += acc;

        if (Down && speed > -vitessemax)
            if (speed > 0) speed -= dec;
            else  speed -= acc;

        if (!Up && !Down)
            if (speed - dec > 0) speed -= dec;
            else if (speed + dec < 0) speed += dec;
            else speed = 0;

        if (Right && speed != 0)  angle += turnSpeed * speed / vitessemax; // tourner a droite 
        if (Left && speed != 0)   angle -= turnSpeed * speed / vitessemax; // tourner a gauche 

        car[0].speed = speed;
        car[0].angle = angle;

        for (int i = 0; i < N; i++) car[i].move();
        for (int i = 1; i < N; i++) car[i].direction();

        //contact des voitures
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
            {
                int dx = 0, dy = 0;
                while (dx * dx + dy * dy < 4 * R * R)
                {
                    car[i].x += dx / 20;
                    car[i].y += dy / 20;
                    car[j].x -= dx / 20;
                    car[j].y -= dy / 20;
                    dx = car[i].x - car[j].x;
                    dy = car[i].y - car[j].y;
                    if (!dx && !dy) break;
                }
            }


        // centrer l'ecran sur la voiture du joueur 
        int offsetX = 0, offsetY = 0;


        if (car[0].x > 320)
            offsetX = car[0].x - 320;
        if (car[0].y > 240)
            offsetY = car[0].y - 240;

        app.clear(Color::Black);// ajouter un arriere plan noir 


        sBackground.setPosition(-offsetX, -offsetY);
        app.draw(sBackground);
        sf::Sprite backgroundSprite(t1);
        sf::Sprite carSprite(t2);
        
        for (int i = 1; i < N; i++)
        {
            int dx = car[i].x - car[0].x;
            int dy = car[i].y - car[0].y;
            float distance = std::sqrt(dx * dx + dy * dy);
            if (distance < 2 * R)
            {
                contactsound.play();
            }
        }



        Color colors[10] = { Color::White/*voiture du joueur*/, Color::Blue, Color::Magenta, Color::Cyan, Color::White, Color::Yellow , Color::Cyan ,Color::Green , Color::Cyan ,Color::Red }; // couleur voitures (max 10 ) 

        for (int i = 0; i < N; i++)
        {
            sCar.setPosition(car[i].x - offsetX, car[i].y - offsetY);
            /* utiliser les coordonnés du voiture de joueur comme reference pour calculer la position des autre voiture
            sur l'arriere plan c-a-d la voiture principale est fixe mais l'arriere plan varie de meme les autres voitures */
            sCar.setRotation(car[i].angle * 180 / 3.141593);
            sCar.setColor(colors[i]); // defenir les couleurs de chaque voiture 
            app.draw(sCar);
        }

        app.display();
    }

    return 0;
}
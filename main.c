/*
    A mineSweeper game made with raylib and raymath
*/

#include <stdio.h>
#include <time.h>

#include "../../../src/raylib.h"
#include "../../../src/raymath.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// Define the Cols and rows of the grid
#define COLS 10 // the number of columns
#define ROWS 10 // the number of rows

// Local variables
const int screenWidth = 400;
const int screenHeight = 400;

// have a variable to divide the screen into 10x10 grid
int gridWidth = screenWidth / COLS;   // the width of each cell
int gridHeight = screenHeight / ROWS; // the height of each cell

// a struct for the cells
typedef struct Cell
{
    int i;              // the index of the cell in the grid
    int j;              // the index of the cell in the grid
    bool containsMine;  // if the cell contains a mine
    bool revealed;      // if the cell is revealed
    bool flagged;       // if the cell is flagged
    int nearbyMines;    // the number of mines nearby
} Cell;

// a two dimensional array of cells
Cell grid[COLS][ROWS];  // the grid of cells

// texture for the flag
Texture2D flagSprite;

// Local Functions Declaration
//----------------------------------------------------------------------------------
void CellDraw(Cell cell);           // Draw a cell
bool IsIndexValid(int i, int j);    // Check if the index is valid

void GridInit(void);                // Initialize the grid

void CallReveal(int i, int j);      // Call the reveal function
int CellCountMines(int i, int j);   // Count the number of nearby mines per cell
void cellFlag(int i, int j);         // Flag the cell


static void UpdateDrawFrame(void);  // Update and draw one frame

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
   
    srand(time(0));

    InitWindow(screenWidth, screenHeight, "MineSweeper");

    GridInit(); // Initialize the grid
    //--------------------------------------------------------------------------------------

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    // unload the flag texture
    UnloadTexture(flagSprite);

    return 0;
}

// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    
    // Check if the left mouse button is pressed and if the index is valid 
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) // If the left mouse button is pressed
    {
        Vector2 mousePos = GetMousePosition();  // Get the mouse position
        int indexI = mousePos.x / gridWidth;    // Get the index of the cell in the grid
        int indexJ = mousePos.y / gridHeight;   // Get the index of the cell in the grid

        if (IsIndexValid(indexI, indexJ))       // Check if the index is valid
        {
            CallReveal(indexI, indexJ);         // Call the reveal function
        }
    }
    else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) // If the right mouse button is pressed
    {
        Vector2 mousePos = GetMousePosition();  // Get the mouse position
        int indexI = mousePos.x / gridWidth;    // Get the index of the cell in the grid
        int indexJ = mousePos.y / gridHeight;   // Get the index of the cell in the grid

        if (IsIndexValid(indexI, indexJ))       // Check if the index is valid
        {
            cellFlag(indexI, indexJ);           // Call the flag function

          //  grid[indexI][indexJ].flagged = !grid[indexI][indexJ].flagged; // Toggle the flag
        }
    }
    
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    // Draw the grid
    for (int i = 0; i < COLS; i++)              // Loop through the columns
    {
        for (int j = 0; j < ROWS; j++)          // Loop through the rows
        {
            CellDraw(grid[i][j]);               // Draw the cell
        }
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
}

// Initialize the grid
void GridInit(void)
{
    flagSprite = LoadTexture("resources/flag.png"); // Load the flag texture

    // Initialize the grid
    for (int i = 0; i < COLS; i++)              // Loop through the columns
    {
        for (int j = 0; j < ROWS; j++)          // Loop through the rows
        {
            grid[i][j] = (Cell)                 // Initialize the cell
                {
                    .i = i,                     // Set the index of the cell in the grid
                    .j = j,                     // Set the index of the cell in the grid
                    .containsMine = false,      // Set the cell to not contain a mine
                    .revealed = false,          // Set the cell to not be revealed
                    .nearbyMines = -1,           // Set the number of nearby mines to -1
                    .flagged = false            // Set the cell to not be flagged
                };                
        }
    }


    // Place the mines
    int minesToPlace = (int)(ROWS * COLS * 0.1f); // the number of mines to place

    while (minesToPlace > 0)                    // Loop until all the mines are placed
    {
        int i = rand() % COLS;                  // Get a random index for the column
        int j = rand() % ROWS;                  // Get a random index for the row

        if (!grid[i][j].containsMine)           // if the cell doesn't contain a mine
        {
            grid[i][j].containsMine = true;     // Set the cell to contain a mine
            minesToPlace--;                     // Decrement the number of mines to place
        }
    }

    //  Count the number of nearby mines per cell
    for (int i = 0; i < COLS; i++)              // Loop through the columns
    {
        for (int j = 0; j < ROWS; j++)          // Loop through the rows
        {
            grid[i][j].nearbyMines = CellCountMines(i, j); // Count the number of nearby mines per cell
        }
    }  
}

// Draw the cells
void CellDraw(Cell cell)
{
    // if revealed, and if contains mine, draw a red square else draw a light gray square
    if (cell.revealed) // if revealed
    {
        if (cell.containsMine) // if contains mine
        {
            DrawRectangle(cell.i * gridWidth, cell.j * gridHeight, gridWidth, gridHeight, RED); // draw a red square
        }
        else // if doesn't contain mine
        {
            DrawRectangle(cell.i * gridWidth, cell.j * gridHeight, gridWidth, gridHeight, LIGHTGRAY); // draw a light gray square
            // if the clicked on cell doesn't contain a mine, draw the number of nearby mines
            if (cell.nearbyMines > 0)
            {
                DrawText(TextFormat("%i", cell.nearbyMines), cell.i * gridWidth + 10, cell.j * gridHeight + 10, 20, BLACK);
            }
        }
    }
    // else if flagged, draw flag sprite
    else if (cell.flagged)
    {
        Rectangle source = { 0, 0, flagSprite.width, flagSprite.height }; // the source rectangle
        Rectangle dest = { cell.i * gridWidth, cell.j * gridHeight, gridWidth, gridHeight }; // the destination rectangle
        Vector2 origin = { 0, 0 }; // the origin of the texture

        DrawTexturePro(flagSprite, source, dest, origin, 0.0f, Fade(WHITE, 0.3f)); // draw the flag sprite
    }

    // Draw the cell lines
    DrawRectangleLines(cell.i * gridWidth, cell.j * gridHeight, gridWidth, gridHeight, BLACK);
}

// Check if the index is valid
bool IsIndexValid(int i, int j)
{
    return i >= 0 && i < COLS && j >= 0 && j < ROWS;    // return true if the index is valid
}

// reveal the cell and if it contains a mine, lose the game
void CallReveal(int i, int j)
{
    // is flagged, return
    if (grid[i][j].flagged)
    {
        return;
    }
    
    grid[i][j].revealed = true;     // reveal the cell

    if (grid[i][j].containsMine)    // if the cell contains a mine
    {
        // Lose the game
    }
    else // if the cell doesn't contain a mine
    {
        // TODO: play a sound.
    }
}

// flag the cell
void cellFlag(int i, int j)
{
    // if revealed, return
    if (grid[i][j].revealed)
    {
        return;
    }

    grid[i][j].flagged = !grid[i][j].flagged; // Toggle the flag
}

// Count the number of nearby mines per cell
int CellCountMines(int i, int j)
{
    int count = 0; // the number of nearby mines

    // Loop through the nearby cells
    for (int x = -1; x <= 1; x++) // Loop through the columns
    {
        for (int y = -1; y <= 1; y++) // Loop through the rows
        {
            int nearbyI = i + x; // Get the nearby index of the cell in the grid
            int nearbyJ = j + y; // Get the nearby index of the cell in the grid

            if (IsIndexValid(nearbyI, nearbyJ)) // Check if the index is valid
            {
                if (grid[nearbyI][nearbyJ].containsMine) // if the cell contains a mine
                {
                    count++; // Increment the number of nearby mines
                }
            }
        }
    }

    return count; // return the number of nearby mines
}
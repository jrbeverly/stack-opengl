#include "Stack.hpp"

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/OpenGLImport.hpp"

#include <iostream>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

// Dimensions of the grid
static const size_t DIM = 16;

// Height maximum of the grid
static const size_t MAX_HEIGHT = 5;

// Scale bounds on the scale
static const float SCALE_LOWER = 0.5f;
static const float SCALE_UPPER = 2.0f;

//----------------------------------------------------------------------------------------
// Constructor
Stack::Stack()
: current_col( 0 ),
m_grid( DIM )
{
    colour[0] = 0.0f;
    colour[1] = 0.0f;
    colour[2] = 0.0f;
}

//----------------------------------------------------------------------------------------
// Destructor
Stack::~Stack()
{

}

//----------------------------------------------------------------------------------------
/*
* Restricts a value to be within a specified range.
*/
int math_clamp(int value, int low, int high)
{
    if (value < low) { return low; }
    if (value > high) { return high; }
    return value;
}

//----------------------------------------------------------------------------------------
/*
* Restricts a value to be within a specified range.
*/
float math_clamp(float value, float low, float high)
{
    if (value < low) { return low; }
    if (value > high) { return high; }
    return value;
}

//----------------------------------------------------------------------------------------
/*
* Called once, at program start.
*/
void Stack::init()
{
    // Set the background colour.
    glClearColor( 0.3, 0.5, 0.7, 1.0 );

    // Build the shader
    m_shader.generateProgramObject();
    m_shader.attachVertexShader( getAssetFilePath( "VertexShader.vs" ).c_str() );
    m_shader.attachFragmentShader( getAssetFilePath( "FragmentShader.fs" ).c_str() );
    m_shader.link();

    // Set up the uniforms
    P_uni = m_shader.getUniformLocation( "P" );
    V_uni = m_shader.getUniformLocation( "V" );
    M_uni = m_shader.getUniformLocation( "M" );
    col_uni = m_shader.getUniformLocation( "colour" );

    // Initialize application state and object buffers
    initState();
    initGrid();
    initCube();

    // Set up initial view and projection matrices (need to do this here,
    // since it depends on the GLFW window being set up correctly).
    view = glm::lookAt(
      glm::vec3( 0.0f, float(DIM) * 2.0 * M_SQRT1_2, float(DIM) * 2.0 * M_SQRT1_2 ),
      glm::vec3( 0.0f, 0.0f, 0.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f ) );

    proj = glm::perspective(
      glm::radians( 45.0f ),
      float( m_framebufferWidth ) / float( m_framebufferHeight ),
      1.0f, 1000.0f );
}

//----------------------------------------------------------------------------------------
// Initializes the state of the application to the default values
void Stack::initState()
{
    // Initializes the grid colours (RGB)
    grid_colours[0] = vec3(1.0f, 0.0f, 0.0f);     // Red
    grid_colours[1] = vec3(0.0f, 0.74f, 1.0f);    // Blue
    grid_colours[2] = vec3(0.13f, 0.54f, 0.13f);  // Green
    grid_colours[3] = vec3(1.0f, 0.54f, 0.0f);    // Orange
    grid_colours[4] = vec3(1.0f, 1.0f, 0.0f);     // Yellow
    grid_colours[5] = vec3(0.54f, 0.0f, 0.54f);   // Purple
    grid_colours[6] = vec3(0.0f, 1.0f, 1.0f);     // Cyan
    grid_colours[7] = vec3(0.66f, 0.66f, 0.66f);  // Gray
    grid_colours[8] = vec3(0.72f, 0.52f, 0.04f);   // Brown

    // Reset the scale
    current_scale = 1.0f;
    current_angle = 0.0f;
    current_col = 0;

    // Set the grid to default height + colour
    m_grid.reset(10);

    // Active cell
    grid_pos_x = 0;
    grid_pos_y = 0;

    // Mouse and key state
    isShiftDown = false;
    isMouseDown = false;

    glm::vec3 val = grid_colours[current_col];
    colour[0] = val.x;
    colour[1] = val.y;
    colour[2] = val.z;
}

// Initializes the grid of the application
void Stack::initGrid()
{
    size_t vcount = 3 * 2 * 2 * (DIM + 3);

    float vertices[ vcount ];
    size_t ct = 0;
    for( int idx = 0; idx < DIM+3; ++idx ) {
        vertices[ ct ] = -1;
        vertices[ ct+1 ] = 0;
        vertices[ ct+2 ] = idx-1;
        vertices[ ct+3 ] = DIM+1;
        vertices[ ct+4 ] = 0;
        vertices[ ct+5 ] = idx-1;
        ct += 6;

        vertices[ ct ] = idx-1;
        vertices[ ct+1 ] = 0;
        vertices[ ct+2 ] = -1;
        vertices[ ct+3 ] = idx-1;
        vertices[ ct+4 ] = 0;
        vertices[ ct+5 ] = DIM+1;
        ct += 6;
    }

    // Create the vertex array to record buffer assignments.
    glGenVertexArrays( 1, &m_grid_vao );
    glBindVertexArray( m_grid_vao );

    // Create the cube vertex buffer
    glGenBuffers( 1, &m_grid_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
    glBufferData( GL_ARRAY_BUFFER, vcount * sizeof(float), vertices, GL_STATIC_DRAW );

    // Specify the means of extracting the position values properly.
    GLint posAttrib = m_shader.getAttribLocation( "position" );
    glEnableVertexAttribArray( posAttrib );
    glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

    // Reset state
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    // Check for errors
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Initializes the cube vertex and index data
void Stack::initCube()
{
    // Vertices that define the cube
    size_t vcount = 8;
    glm::vec3 vertices[] = {
      {  0.0f, 0.0f, 1.0f },
      {  1.0f, 0.0f, 1.0f },
      {  1.0f, 1.0f, 1.0f },
      {  0.0f, 1.0f, 1.0f },
      {  0.0f, 0.0f, 0.0f },
      {  1.0f, 0.0f, 0.0f },
      {  1.0f, 1.0f, 0.0f },
      {  0.0f, 1.0f, 0.0f },
    };

    // Indices that define the triangles that construct the cube
    size_t icount = 12 * 3;
    m_cube_icount = icount;
    GLint indices[] = {
        0, 1, 2, 2, 3, 0,
        3, 2, 6, 6, 7, 3,
        7, 6, 5, 5, 4, 7,
        4, 0, 3, 3, 7, 4,
        0, 1, 5, 5, 4, 0,
        1, 5, 6, 6, 2, 1
    };

    // Setup the vertex array
    glGenVertexArrays(1, &m_cube_vao);
    glBindVertexArray(m_cube_vao);

    // Setup the vertices of the cube
    glGenBuffers(1, &m_cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, vcount * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    // Specify the means of extracting the position values properly.
    GLint posAttrib = m_shader.getAttribLocation( "position" );
    glEnableVertexAttribArray( posAttrib );
    glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

    // Setup indices for the cube
    glGenBuffers( 1, &m_cube_ibo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_cube_ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, icount * sizeof(GLint), &indices[0], GL_STATIC_DRAW);

    // Reset state
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
* Called once per frame, before guiLogic().
*/
void Stack::appLogic()
{
    // Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
* Called once per frame, after appLogic(), but before the draw() method.
*/
void Stack::guiLogic()
{
    // We already know there's only going to be one window, so for
    // simplicity we'll store button states in static local variables.
    // If there was ever a possibility of having multiple instances of
    // Stack running simultaneously, this would break; you'd want to make
    // this into instance fields of Stack.
    static bool showTestWindow(false);
    static bool showDebugWindow(true);

    ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
    float opacity(0.5f);

    ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
    if( ImGui::Button( "Quit Application" ) ) {
        glfwSetWindowShouldClose(m_window, GL_TRUE);
    }

    /// RADIO BUTTON CODE BEGIN

    // Display the colour options in the window
    ImGui::Text("Colour Options:");

    bool isSelected = false;

    // The radio buttons that define colours
    // Each ID corresponds to a RGB colour defined in grid_colours
    if (ImGui::RadioButton("Color 1", &current_col, 0)) { isSelected = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Color 2", &current_col, 2)) { isSelected = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Color 3", &current_col, 5)) { isSelected = true; }

    if (ImGui::RadioButton("Color 4", &current_col, 1)) { isSelected = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Color 5", &current_col, 8)) { isSelected = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Color 6", &current_col, 4)) { isSelected = true; }

    if (ImGui::RadioButton("Color 7", &current_col, 6)) { isSelected = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Color 8", &current_col, 7)) { isSelected = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Color 9", &current_col, 3)) { isSelected = true; }

    // If a button is selected, check if cell has values and updated colour
    if (isSelected)
    {
        // Get height, check if has values, then set colour
        int height = m_grid.getHeight(grid_pos_x, grid_pos_y);
        if (height > 0)
        {
            m_grid.setColour(grid_pos_x, grid_pos_y, current_col);
        }
    }

    // Sets the current default colour value for the display
    glm::vec3 val = grid_colours[current_col];
    colour[0] = val.x;
    colour[1] = val.y;
    colour[2] = val.z;

    /// RADIO BUTTON CODE END

    // Create a colour display
    ImGui::ColorEdit3( "##Colour", colour );

    /// RGB HANDLING CODE BEGIN

    ImGui::SliderFloat("R", &grid_colours[current_col][0], 0.0f, 1.0f);
    ImGui::SliderFloat("G", &grid_colours[current_col][1], 0.0f, 1.0f);
    ImGui::SliderFloat("B", &grid_colours[current_col][2], 0.0f, 1.0f);

    /// RGB HANDING CODE END

    // Framerate text
    ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

    ImGui::End();

    if( showTestWindow ) {
        ImGui::ShowTestWindow( &showTestWindow );
    }
}

//----------------------------------------------------------------------------------------
/*
* Called once per frame, after guiLogic().
*/
void Stack::draw()
{
    // Create a global transformation for the model (centre it).
    mat4 W;
    W = glm::rotate( W, glm::radians(current_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );
    W = glm::scale( W, vec3( current_scale, current_scale, current_scale ) );

    /// A note on the above:
    // First we rotate about the Up-axis at the origin
    // Then we shift the grid to be center from camera view
    // Thus we will rotate based on the center of the grid (approx)
    // Then we scale the outcome

    m_shader.enable();

    // Enable the depth test
    glEnable( GL_DEPTH_TEST );

    // Set the matrix
    glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
    glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
    glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

    // Draw the grid for now.
    glBindVertexArray( m_grid_vao );
    glUniform3f( col_uni, 1, 1, 1 );
    glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

    /// CUBE CODE BEGIN
    // A note on drawing code:
    // Code here uses a very inefficient approach to drawing all the cubes
    // by sending a call for each cube (+ one for wireframe) to the GPU
    // when the more ideal solution is to send a matrix (or vector) to GPU
    // that can be used to define drawing of each cube.  This option was chosen
    // as this is not a performance critical application

    // Set vertex and index buffer for cubes
    glBindVertexArray( m_cube_vao );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cube_ibo);

    // Iterate through the grid
    for(int dx = 0; dx < DIM; dx++)
    {
      for(int dy = 0; dy < DIM; dy++)
      {
        // If height is 0 then no drawing necessary
        if (m_grid.getHeight(dx, dy) == 0)
        {
            continue;
        }

        // Get values from grid
        int height = m_grid.getHeight(dx, dy);
        int colour = m_grid.getColour(dx, dy);
        vec3 vcol = grid_colours[colour];

        // Draw each of the cubes
        for(int ch = 0; ch < height; ch++)
        {
          // Set the local model values
          mat4 local_w = glm::translate( W, vec3( dx * 1.0f, ch * 1.0f, dy * 1.0f ) );
          glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( local_w ) );

          // Set color and draw points
          glUniform3f( col_uni, vcol.x, vcol.y, vcol.z);
          glDrawElements(GL_TRIANGLES, m_cube_icount, GL_UNSIGNED_INT, 0);

          glUniform3f( col_uni, 0.0f, 0.0f, 0.0f);
          glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
          glDrawElements(GL_TRIANGLES, m_cube_icount, GL_UNSIGNED_INT, 0);
          glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }
      }
    }

    /// CUBE CODE END

    // Disable the depth test
    glDisable( GL_DEPTH_TEST );

    /// MARKER CODE BEGIN

    // Set the marker model matrix
    mat4 local_w;
    local_w = glm::translate( W, vec3( grid_pos_x * 1.0f, 0.0f, grid_pos_y * 1.0f ) );
    local_w = glm::scale( local_w, vec3( 1.0f, 6.0f, 1.0f ) );

    glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( local_w ) );

    // Set color and draw points
    glUniform3f( col_uni, 0.0f, 0.0f, 0.0f);

    // Draw as wireframe
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDrawElements(GL_TRIANGLES, m_cube_icount, GL_UNSIGNED_INT, 0);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    /// MARKER CODE END

    // Highlight the active square.
    m_shader.disable();

    // Restore defaults
    glBindVertexArray( 0 );

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
* Called once, after program is signaled to terminate.
*/
void Stack::cleanup()
{
    glDeleteBuffers(1, &m_grid_vbo);

    glDeleteBuffers(1, &m_cube_vbo);
    glDeleteBuffers(1, &m_cube_ibo);
}

//----------------------------------------------------------------------------------------
/*
* Event handler.  Handles cursor entering the window area events.
*/
bool Stack::cursorEnterWindowEvent (int entered) {
    bool eventHandled(false);

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
* Event handler.  Handles mouse cursor movement events.
*/
bool Stack::mouseMoveEvent(double xPos, double yPos)
{
    bool eventHandled(false);
    const float degree = 0.1f;

    if (!ImGui::IsMouseHoveringAnyWindow() && isMouseDown) {
        // Get the change in mouse position
        double xdist = xPos - prev_mouse_x;
        current_angle += xdist * degree;

        eventHandled = true;
    }

    prev_mouse_x = xPos;
    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
* Event handler.  Handles mouse button events.
*/
bool Stack::mouseButtonInputEvent(int button, int actions, int mods) {
    bool eventHandled(false);

    isMouseDown = false;
    if (!ImGui::IsMouseHoveringAnyWindow()) {
        // The user clicked in the window.  If it's the left
        // mouse button, initiate a rotation.

        if (actions == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
        {
            isMouseDown = true;
            eventHandled = true;
        }
    }

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
* Event handler.  Handles mouse scroll wheel events.
*/
bool Stack::mouseScrollEvent(double xOffSet, double yOffSet) {
    bool eventHandled(true);
    const float degree = 0.05f;

    // Zoom in or out.
    float new_scale = current_scale + (degree * xOffSet);

    // Set the new current scale
    current_scale = math_clamp(new_scale, SCALE_LOWER, SCALE_UPPER);

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
* Event handler.  Handles window resize events.
*/
bool Stack::windowResizeEvent(int width, int height) {
    bool eventHandled(false);

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
* Event handler.  Handles key input events.
*/
bool Stack::keyInputEvent(int key, int action, int mods) {
    bool eventHandled(false);
    int moveXAxis(0);
    int moveYAxis(0);

    if( action == GLFW_RELEASE ) {

        // When shift is released, set boolean to false
        if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
            isShiftDown = false;
            eventHandled = true;
        }
    }

    if( action == GLFW_PRESS ) {

        // Quit the application by telling glfw to quit
        if (key == GLFW_KEY_Q) {
            glfwSetWindowShouldClose(m_window, GL_TRUE);
            eventHandled = true;
        }

        // Reset the grid to the base state
        if (key == GLFW_KEY_R) {
            initState();
            eventHandled = true;
        }

        // Add a block to the current grid position
        if (key == GLFW_KEY_SPACE) {
            incrementCell(grid_pos_x, grid_pos_y);
            eventHandled = true;
        }

        // Remove a block from the current grid position
        if (key == GLFW_KEY_BACKSPACE) {
            decrementCell(grid_pos_x, grid_pos_y);
            eventHandled = true;
        }

        // Determine if the copy action should be performed
        if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
            isShiftDown = true;
            eventHandled = true;
        }

        // Set the grid movement degree for key left
        if (key == GLFW_KEY_LEFT) {
            moveXAxis -= 1;
            eventHandled = true;
        }

        // Set the grid movement degree for key right
        if (key == GLFW_KEY_RIGHT) {
            moveXAxis += 1;
            eventHandled = true;
        }

        // Set the grid movement degree for key up
        if (key == GLFW_KEY_UP) {
            moveYAxis -= 1;
            eventHandled = true;
        }

        // Set the grid movement degree for key down
        if (key == GLFW_KEY_DOWN) {
            moveYAxis += 1;
            eventHandled = true;
        }

        // Perform actions of the values
        if (moveXAxis != 0 || moveYAxis != 0)
        {
            // Array is size DIM, so positions are limited to [0, DIM - 1]
            int newX = math_clamp(grid_pos_x + moveXAxis, 0, DIM - 1);
            int newY = math_clamp(grid_pos_y + moveYAxis, 0, DIM - 1);

            // If the shift key is pressed we must copy the grid
            // copy the current grid to the new position
            if (isShiftDown)
            {
                copyCell(grid_pos_x, grid_pos_y, newX, newY);
            }

            // Sets the active cell
            setActiveCell(newX, newY);
        }
    }

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
* Decreases the height of the specified cell.
*/
void Stack::decrementCell(int cellX, int cellY){
    // Get the new height of the cell
    int height = m_grid.getHeight(cellX, cellY) - 1;
    height = math_clamp(height, 0, MAX_HEIGHT);

    // Set colour and height
    m_grid.setHeight(cellX, cellY, height);
    m_grid.setColour(cellX, cellY, current_col);
}

//----------------------------------------------------------------------------------------
/*
* Increases the height of the specified cell.
*/
void Stack::incrementCell(int cellX, int cellY){
    // Get the new height of the cell
    int height = m_grid.getHeight(cellX, cellY) + 1;
    height = math_clamp(height, 0, MAX_HEIGHT);

    // Set colour and height
    m_grid.setHeight(cellX, cellY, height);
    m_grid.setColour(cellX, cellY, current_col);
}

//----------------------------------------------------------------------------------------
/*
* Copies the properties of the specified cell.
*/
void Stack::copyCell(int sourceX, int sourceY, int destX, int destY){
    // Get source cell details
    int srcHeight = m_grid.getHeight(sourceX, sourceY);
    int srcColour = m_grid.getColour(sourceX, sourceY);

    // Set the height and colour to next cell
    m_grid.setHeight(destX, destY, srcHeight);
    m_grid.setColour(destX, destY, srcColour);
}

//----------------------------------------------------------------------------------------
/*
* Sets the active cell of the application.
*/
void Stack::setActiveCell(int cellX, int cellY) {
    grid_pos_x = cellX;
    grid_pos_y = cellY;
}

#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "grid.hpp"

class Stack : public CS488Window {
public:
	Stack();
	virtual ~Stack();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

private:
	// Initialize the application
	void initGrid();
	void initCube();
	void initState();

	// Increment and decrement of cell heights
	void decrementCell(int cellX, int cellY);
	void incrementCell(int cellX, int cellY);

	// Copies the specified cell to the specified cell
	void copyCell(int sourceX, int sourceY, int destX, int destY);

	// Sets the active cell of the application
	void setActiveCell(int cellX, int cellY);

	// Fields related to the grid
	Grid m_grid;

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Fields related to cube geometry.
	GLuint m_cube_vao; // Vertex Array Object
	GLuint m_cube_vbo; // Vertex Buffer Object
	GLuint m_cube_ibo; // Index Buffer Object
	GLint m_cube_icount; // Index Buffer Count

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	// Camera positions
	glm::vec3 m_position;
	glm::vec3 m_direction;

	// Grid positions
	int grid_pos_x;
	int grid_pos_y;

	// Mouse positions
	bool isMouseDown;
	bool isShiftDown;
	double prev_mouse_x;

	// State values
	float current_scale;
	float current_angle;

	glm::vec3 grid_colours[9];
	float colour[3];
	int current_col;
};

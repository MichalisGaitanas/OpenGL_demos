#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>
#include<string>
#include<array>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"
#include"../include/camera.hpp"

const double pi = 3.1415926535897932384626433832795;

//Camera object instantiation. We make it global so that the glfw callback 'cursor_pos_callback()' (see later) can
//have access to it. This is just for demo. At a bigger project, we would use glfwSetWindowUserPointer(...) to encapsulate
//any variable within the specific context of the window.
camera cam(glm::vec3(0.0f, -5.0f, 0.0f),
           glm::vec3(0.0f,0.0f,1.0f),
           90.0f,
           0.0f,
           2.0f,
           5.0f,
           0.05f,
           45.0f );

double time_tick; //Elapsed time per frame update.

double xpos_previous, ypos_previous;
bool first_time_entered_the_window = true;

bool cursor_visible = false;

int win_width = 1200, win_height = 900;

//For 'continuous' events, i.e. at every frame in the while() loop.
void event_tick(GLFWwindow *win)
{
    bool move_key_pressed = false;
    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, cam.front);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, -cam.front);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, cam.right);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, -cam.right);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, cam.world_up);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, -cam.world_up);
        move_key_pressed = true;
    }

    //If no keys are pressed, decelerate.
    if (!move_key_pressed)
        cam.decelerate(time_tick);
}

//For discrete keyboard events.
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

//When a mouse button is pressed, do the following :
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    //Toggle cursor visibility via the mouse right click.
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        cursor_visible = !cursor_visible;
        if (cursor_visible)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            first_time_entered_the_window = true;
        }
    }
}

//When the mouse moves, do the following :
void cursor_pos_callback(GLFWwindow *win, double xpos, double ypos)
{
    if (cursor_visible)
        return;

    if (first_time_entered_the_window)
    {
        xpos_previous = xpos;
        ypos_previous = ypos;
        first_time_entered_the_window = false;
    }

    double xoffset = xpos - xpos_previous;
    double yoffset = ypos - ypos_previous;

    xpos_previous = xpos;
    ypos_previous = ypos;

    cam.rotate(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *win, double xoffset, double yoffset)
{
    cam.zoom((double)yoffset);
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
}


typedef std::array<double, 2> dvec2;
typedef std::array<double, 3> dvec3;
typedef std::array<double, 4> dvec4;
typedef std::array<double, 20> dvec20;
typedef std::array<dvec3, 3> dmat3;

double G,M1,M2; //Gravity constant and asteroid masses.
dmat3 I1,I2; //Moment of inertia tensors of the asteroids.
double duration, dt; //Integration duration and method's step.



/* Overload some operators to make our life easier. */

//Define the operation v1 + v2 (v1,v2 are 3x1 vectors).
dvec3 operator+(const dvec3 &v1, const dvec3 &v2)
{
    return {v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]};
}

//Define the operation v1 + v2 (v1,v2 are 4x1 vectors).
dvec4 operator+(const dvec4 &v1, const dvec4 &v2)
{
    return {v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2], v1[3] + v2[3]};
}

//Define the operation v1 - v2 (v1,v2 are 3x1 vectors).
dvec3 operator-(const dvec3 &v1, const dvec3 &v2)
{
    return {v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]};
}

//Define the operation -v (v is 3x1 vector).
dvec3 operator-(const dvec3 &v)
{
    return {-v[0], -v[1], -v[2]};
}

//Define the operation c*v (c is scalar, v is 3x1 vector).
dvec3 operator*(const double c, const dvec3 &v)
{
    return {c*v[0], c*v[1], c*v[2]};
}

//Define the operation v*c (v is 3x1 vector, c is scalar).
dvec3 operator*(const dvec3 &v, const double c)
{
    return {v[0]*c, v[1]*c, v[2]*c};
}

//Define the operation c*v (c is scalar, v is 4x1 vector).
dvec4 operator*(const double c, const dvec4 &v)
{
    return {c*v[0], c*v[1], c*v[2], c*v[3]};
}

//Define the operation v*c (v is 4x1 vector, c is scalar).
dvec4 operator*(const dvec4 &v, const double c)
{
    return {v[0]*c, v[1]*c, v[2]*c, v[3]*c};
}

//Define the operation v/c (v is c is 3x1 vector, c is scalar).
dvec3 operator/(const dvec3 &v, const double c)
{
    return {v[0]/c, v[1]/c, v[2]/c};
}

//Define the operation v/c (v is c is 4x1 vector, c is scalar).
dvec4 operator/(const dvec4 &v, const double c)
{
    return {v[0]/c, v[1]/c, v[2]/c, v[3]/c};
}

/* End of operator overloading. */



/* Define some algebraic routines. */

//Transpose of a 3x3 matrix.
dmat3 transpose(const dmat3 &A)
{
    return {{{A[0][0], A[1][0], A[2][0]},
             {A[0][1], A[1][1], A[2][1]},
             {A[0][2], A[1][2], A[2][2]}}};
}

//Matrix-vector product A*v of a 3x3 matrix and a 3x1 vector.
dvec3 dot(const dmat3 &A, const dvec3 &v)
{
    return { A[0][0]*v[0] + A[0][1]*v[1] + A[0][2]*v[2],
             A[1][0]*v[0] + A[1][1]*v[1] + A[1][2]*v[2],
             A[2][0]*v[0] + A[2][1]*v[1] + A[2][2]*v[2] };
}

//Dot product of 2 3-coord vectors.
double dot(const dvec3 &v1, const dvec3 &v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

//Vector-matrix product v*A of a 1x3 vector and a 3x3 matrix.
dvec3 dot(const dvec3 &v, const dmat3 &A)
{
    return { v[0]*A[0][0] + v[1]*A[1][0] + v[2]*A[2][0],
             v[0]*A[0][1] + v[1]*A[1][1] + v[2]*A[2][1],
             v[0]*A[0][2] + v[1]*A[1][2] + v[2]*A[2][2] };
}

//Cross product of 2 3-coord vectors.
dvec3 cross(const dvec3 &v1, const dvec3 &v2)
{
    return { v1[1]*v2[2] - v1[2]*v2[1],
             v1[2]*v2[0] - v1[0]*v2[2],
             v1[0]*v2[1] - v1[1]*v2[0] };
}

//Convert a quaternion to a unit one.
dvec4 quat2unit(const dvec4 &q)
{
    return q/sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
}

//Convert a unit quaternion to Euler angles (roll, pitch, yaw).
//Angles are in RAD!
dvec3 quat2ang(const dvec4 &q)
{
    //roll
    double roll = atan2( 2.0*(q[2]*q[3] + q[0]*q[1]), q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3] );
    //pitch
    double pitch, coeff = q[1]*q[3] - q[0]*q[2];
    if (-2.0*coeff >= 1.0)
        pitch = 2.0*pi;
    else if (-2.0*coeff <= -1.0)
        pitch = -2.0*pi;
    else
        pitch = asin(-2.0*coeff);
    //yaw
    double yaw = atan2( 2.0*(q[1]*q[2] + q[0]*q[3]), q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3] );
    return {roll, pitch, yaw};
}

//Convert a unit quaternion to rotation matrix (homogeneous expression).
dmat3 quat2mat(const dvec4 &q)
{
    double a11 = q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3];
    double a12 = 2.0*(q[1]*q[2] - q[0]*q[3]);
    double a13 = 2.0*(q[1]*q[3] + q[0]*q[2]);
    double a21 = 2.0*(q[1]*q[2] + q[0]*q[3]);
    double a22 = q[0]*q[0] - q[1]*q[1] + q[2]*q[2] - q[3]*q[3];
    double a23 = 2.0*(q[2]*q[3] - q[0]*q[1]);
    double a31 = 2.0*(q[1]*q[3] - q[0]*q[2]);
    double a32 = 2.0*(q[2]*q[3] + q[0]*q[1]);
    double a33 = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];
    return {{{a11,a12,a13},
             {a21,a22,a23},
             {a31,a32,a33}}};
}

//Convert a vector from the inertial to the body frame.
dvec3 iner2body(const dvec3 &viner, const dmat3 &A)
{
    return dot(transpose(A), viner);
}

//Convert a vector from the body to the inertial frame.
dvec3 body2iner(const dvec3 &vbody, const dmat3 &A)
{
    return dot(A, vbody);
}

/* End of algebraic routines definition. */


/* Define the physics functions. */

//Quaternion odes rhs (angular velocity w is in the body frame).
dvec4 quat_rhs(const dvec4 &q, const dvec3 &w)
{
    double dq0 = 0.5*(-q[1]*w[0] - q[2]*w[1] - q[3]*w[2]);
    double dq1 = 0.5*( q[0]*w[0] - q[3]*w[1] + q[2]*w[2]);
    double dq2 = 0.5*( q[3]*w[0] + q[0]*w[1] - q[1]*w[2]);
    double dq3 = 0.5*(-q[2]*w[0] + q[1]*w[1] + q[0]*w[2]);
    return {dq0, dq1, dq2, dq3};
}

//Euler odes rhs assuming I[][] is diagonal (principal axes frame).
//Angular velocity w, moment of inertia I and torque tau are in the body frame.
dvec3 euler_rhs(const dvec3 &w, const dmat3 &I, const dvec3 &tau)
{
    double dw0 = (tau[0] + w[1]*w[2]*(I[1][1] - I[2][2]))/I[0][0];
    double dw1 = (tau[1] + w[2]*w[0]*(I[2][2] - I[0][0]))/I[1][1];
    double dw2 = (tau[2] + w[0]*w[1]*(I[0][0] - I[1][1]))/I[2][2];
    return {dw0, dw1, dw2};
}

//Moment of inertia matrix of a triaxial ellipsoid in its principal axes.
dmat3 ell_inertia(const double M, const dvec3 &semiaxes)
{
    double a = semiaxes[0], b = semiaxes[1], c = semiaxes[2];
    double Ix = M*(b*b + c*c)/5.0;
    double Iy = M*(a*a + c*c)/5.0;
    double Iz = M*(a*a + b*b)/5.0;
    return {{{Ix,0.0,0.0},
             {0.0,Iy,0.0},
             {0.0,0.0,Iz}}};
}

//Mutual potential of 2 rigid bodies, assuming inertial integral expansion of order 2 approximation.
double potential(double M1, double M2, const dmat3 &I1, const dmat3 &I2, const dvec3 &r, const dmat3 &A1, const dmat3 &A2)
{
    double I1x = I1[0][0], I1y = I1[1][1], I1z = I1[2][2];
    double I2x = I2[0][0], I2y = I2[1][1], I2z = I2[2][2];

    dvec3 a1 = {A1[0][0], A1[1][0], A1[2][0]};
    dvec3 a2 = {A1[0][1], A1[1][1], A1[2][1]};
    dvec3 a3 = {A1[0][2], A1[1][2], A1[2][2]};

    dvec3 b1 = {A2[0][0], A2[1][0], A2[2][0]};
    dvec3 b2 = {A2[0][1], A2[1][1], A2[2][1]};
    dvec3 b3 = {A2[0][2], A2[1][2], A2[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    dvec3 ru = r/d;

    double l1 = dot(ru,a1);
    double m1 = dot(ru,a2);
    double n1 = dot(ru,a3);

    double l2 = dot(ru,b1);
    double m2 = dot(ru,b2);
    double n2 = dot(ru,b3);

    double V0 = -G*M1*M2/d;

    double V2 = -(G*M2/(2.0*d*d*d))*( (1.0 - 3.0*l1*l1)*I1x + (1.0 - 3.0*m1*m1)*I1y + (1.0 - 3.0*n1*n1)*I1z ) +
                -(G*M1/(2.0*d*d*d))*( (1.0 - 3.0*l2*l2)*I2x + (1.0 - 3.0*m2*m2)*I2y + (1.0 - 3.0*n2*n2)*I2z );

    return V0 + V2;
}

//Mutual force of 2 rigid bodies, assuming inertial integral expansion of order 2 approximation.
dvec3 force(double M1, double M2, const dmat3 &I1, const dmat3 &I2, const dvec3 &r, const dmat3 &A1, const dmat3 &A2)
{
    double I1x = I1[0][0], I1y = I1[1][1], I1z = I1[2][2];
    double I2x = I2[0][0], I2y = I2[1][1], I2z = I2[2][2];

    dvec3 a1 = {A1[0][0], A1[1][0], A1[2][0]};
    dvec3 a2 = {A1[0][1], A1[1][1], A1[2][1]};
    dvec3 a3 = {A1[0][2], A1[1][2], A1[2][2]};

    dvec3 b1 = {A2[0][0], A2[1][0], A2[2][0]};
    dvec3 b2 = {A2[0][1], A2[1][1], A2[2][1]};
    dvec3 b3 = {A2[0][2], A2[1][2], A2[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    dvec3 ru = r/d;

    double l1 = dot(ru,a1);
    double m1 = dot(ru,a2);
    double n1 = dot(ru,a3);

    double l2 = dot(ru,b1);
    double m2 = dot(ru,b2);
    double n2 = dot(ru,b3);

    double V0 = -G*M1*M2/d;

    double V2 = -(G*M2/(2.0*d*d*d))*( (1.0 - 3.0*l1*l1)*I1x + (1.0 - 3.0*m1*m1)*I1y + (1.0 - 3.0*n1*n1)*I1z ) +
                -(G*M1/(2.0*d*d*d))*( (1.0 - 3.0*l2*l2)*I2x + (1.0 - 3.0*m2*m2)*I2y + (1.0 - 3.0*n2*n2)*I2z );

    double dV_dd = -V0/d - 3*V2/d;
    dvec3 dd_dr = ru;

    double dV_dl1 = 3.0*G*I1x*M2*l1/(d*d*d);
    double dV_dm1 = 3.0*G*I1y*M2*m1/(d*d*d);
    double dV_dn1 = 3.0*G*I1z*M2*n1/(d*d*d);
    dvec3 dl1_dr = (a1*d-r*l1)/(d*d);
    dvec3 dm1_dr = (a2*d-r*m1)/(d*d);
    dvec3 dn1_dr = (a3*d-r*n1)/(d*d);

    double dV_dl2 = 3.0*G*I2x*M1*l2/(d*d*d);
    double dV_dm2 = 3.0*G*I2y*M1*m2/(d*d*d);
    double dV_dn2 = 3.0*G*I2z*M1*n2/(d*d*d);
    dvec3 dl2_dr = (b1*d-r*l2)/(d*d);
    dvec3 dm2_dr = (b2*d-r*m2)/(d*d);
    dvec3 dn2_dr = (b3*d-r*n2)/(d*d);

    return -(dV_dd*dd_dr + dV_dl1*dl1_dr + dV_dm1*dm1_dr + dV_dn1*dn1_dr +
                           dV_dl2*dl2_dr + dV_dm2*dm2_dr + dV_dn2*dn2_dr);
}

//Gravity gradient torque of body 1 perceived by body 2, assuming inertial integral expansion of order 2 approximation.
dvec3 torque1(double M2, const dmat3 &I1, const dvec3 &r, const dmat3 &A1)
{
    double I1x = I1[0][0], I1y = I1[1][1], I1z = I1[2][2];

    dvec3 a1 = {A1[0][0], A1[1][0], A1[2][0]};
    dvec3 a2 = {A1[0][1], A1[1][1], A1[2][1]};
    dvec3 a3 = {A1[0][2], A1[1][2], A1[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    dvec3 ru = r/d;

    double l1 = dot(ru,a1);
    double m1 = dot(ru,a2);
    double n1 = dot(ru,a3);

    double dV_dl1 = 3.0*G*I1x*M2*l1/(d*d*d);
    double dV_dm1 = 3.0*G*I1y*M2*m1/(d*d*d);
    double dV_dn1 = 3.0*G*I1z*M2*n1/(d*d*d);

    dvec3 dl1_da1 = ru;
    dvec3 dm1_da2 = ru;
    dvec3 dn1_da3 = ru;

    dvec3 dV_da1 = dV_dl1*dl1_da1;
    dvec3 dV_da2 = dV_dm1*dm1_da2;
    dvec3 dV_da3 = dV_dn1*dn1_da3;

    return -cross(a1, dV_da1) - cross(a2, dV_da2) - cross(a3, dV_da3);
}

//Gravity gradient torque of body 2 perceived by body 1, assuming inertial integral expansion of order 2 approximation.
dvec3 torque2(double M1, const dmat3 &I2, const dvec3 &r, const dmat3 &A2)
{
    double I2x = I2[0][0], I2y = I2[1][1], I2z = I2[2][2];

    dvec3 b1 = {A2[0][0], A2[1][0], A2[2][0]};
    dvec3 b2 = {A2[0][1], A2[1][1], A2[2][1]};
    dvec3 b3 = {A2[0][2], A2[1][2], A2[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    dvec3 ru = r/d;

    double l2 = dot(ru,b1);
    double m2 = dot(ru,b2);
    double n2 = dot(ru,b3);

    double dV_dl2 = 3.0*G*I2x*M1*l2/(d*d*d);
    double dV_dm2 = 3.0*G*I2y*M1*m2/(d*d*d);
    double dV_dn2 = 3.0*G*I2z*M1*n2/(d*d*d);

    dvec3 dl2_db1 = ru;
    dvec3 dm2_db2 = ru;
    dvec3 dn2_db3 = ru;

    dvec3 dV_db1 = dV_dl2*dl2_db1;
    dvec3 dV_db2 = dV_dm2*dm2_db2;
    dvec3 dV_db3 = dV_dn2*dn2_db3;

    return -cross(b1, dV_db1) - cross(b2, dV_db2) - cross(b3, dV_db3);
}

//Total energy and angular momentum of the binary.
dvec2 ener_mom(const dvec20 &state)
{
    dvec3 r   = { state[0], state[1], state[2] };
    dvec3 v   = { state[3], state[4], state[5] };
    dvec4 q1  = { state[6], state[7], state[8],  state[9] };
    dvec3 w1b = { state[10], state[11], state[12] };
    dvec4 q2  = { state[13], state[14], state[15], state[16] };
    dvec3 w2b = { state[17], state[18], state[19] };

    dmat3 A1 = quat2mat(q1);
    dmat3 A2 = quat2mat(q2);

    double E = 0.5*((M1*M2/(M1+M2))*dot(v,v) + dot(dot(w1b,I1), w1b) + dot(dot(w2b,I2), w2b)) + potential(M1,M2, I1,I2, r, A1,A2);
    dvec3 L = (M1*M2/(M1+M2))*cross(r,v) + dot(A1, dot(I1,w1b)) + dot(A2, dot(I2,w2b));

    return {E, sqrt(L[0]*L[0] + L[1]*L[1] + L[2]*L[2])};
}

/* End of physics functions definition. */

/* Write the right hand sides of the differential equations of motion. */

dvec3 fr(const dvec3 &v)
{
    return v;
}

dvec3 fv(const dvec3 &r, const dvec4 &q1, const dvec4 &q2)
{
    dmat3 A1 = quat2mat(q1);
    dmat3 A2 = quat2mat(q2);
    return force(M1,M2, I1,I2, r, A1,A2)/(M1*M2/(M1+M2));
}

dvec4 fq1(const dvec4 &q1, const dvec3 &w1b)
{
    return quat_rhs(q1,w1b);
}

dvec3 fw1(const dvec3 &r, const dvec4 &q1, const dvec3 &w1b)
{
    dmat3 A1 = quat2mat(q1);
    dvec3 tau1i = torque1(M2, I1, r, A1);
    dvec3 tau1b = iner2body(tau1i,A1);
    return euler_rhs(w1b,I1,tau1b);
}

dvec4 fq2(const dvec4 &q2, const dvec3 &w2b)
{
    return quat_rhs(q2,w2b);
}

dvec3 fw2(const dvec3 &r, const dvec4 &q2, const dvec3 &w2b)
{
    dmat3 A2 = quat2mat(q2);
    dvec3 tau2i = torque2(M1, I2, r, A2);
    dvec3 tau2b = iner2body(tau2i,A2);
    return euler_rhs(w2b,I2,tau2b);
}

/* End of right hand sides of the differential equations of motion. */

/* Write the integration method (RK4 scheme). */

void rk4_step(dvec20 &state)
{
    dvec3 r   = { state[0], state[1], state[2] };
    dvec3 v   = { state[3], state[4], state[5] };
    dvec4 q1  = { state[6], state[7], state[8],  state[9] };
    dvec3 w1b = { state[10], state[11], state[12] };
    dvec4 q2  = { state[13], state[14], state[15], state[16] };
    dvec3 w2b = { state[17], state[18], state[19] };

    q1 = quat2unit(q1);
    q2 = quat2unit(q2);

    dvec3 kr = fr(v);
    dvec3 kv = fv(r,q1,q2);
    dvec4 kq1 = fq1(q1,w1b);
    dvec3 kw1b = fw1(r,q1,w1b);
    dvec4 kq2 = fq2(q2,w2b);
    dvec3 kw2b = fw2(r,q2,w2b);

    dvec3 lr = fr(v + 0.5*dt*kv);
    dvec3 lv = fv(r + 0.5*dt*kr, q1 + 0.5*dt*kq1, q2 + 0.5*dt*kq2);
    dvec4 lq1 = fq1(q1 + 0.5*dt*kq1, w1b + 0.5*dt*kw1b);
    dvec3 lw1b = fw1(r + 0.5*dt*kr, q1 + 0.5*dt*kq1, w1b + 0.5*dt*kw1b);
    dvec4 lq2 = fq2(q2 + 0.5*dt*kq2, w2b + 0.5*dt*kw2b);
    dvec3 lw2b = fw2(r + 0.5*dt*kr, q2 + 0.5*dt*kq2, w2b + 0.5*dt*kw2b);

    dvec3 mr = fr(v + 0.5*dt*lv);
    dvec3 mv = fv(r + 0.5*dt*lr, q1 + 0.5*dt*lq1, q2 + 0.5*dt*lq2);
    dvec4 mq1 = fq1(q1 + 0.5*dt*lq1, w1b + 0.5*dt*lw1b);
    dvec3 mw1b = fw1(r + 0.5*dt*lr, q1 + 0.5*dt*lq1, w1b + 0.5*dt*lw1b);
    dvec4 mq2 = fq2(q2 + 0.5*dt*lq2, w2b + 0.5*dt*lw2b);
    dvec3 mw2b = fw2(r + 0.5*dt*lr, q2 + 0.5*dt*lq2, w2b + 0.5*dt*lw2b);

    dvec3 nr = fr(v + dt*mv);
    dvec3 nv = fv(r + dt*mr, q1 + dt*mq1, q2 + dt*mq2);
    dvec4 nq1 = fq1(q1 + dt*mq1, w1b + dt*mw1b);
    dvec3 nw1b = fw1(r + dt*mr, q1 + dt*mq1, w1b + dt*mw1b);
    dvec4 nq2 = fq2(q2 + dt*mq2, w2b + dt*mw2b);
    dvec3 nw2b = fw2(r + dt*mr, q2 + dt*mq2, w2b + dt*mw2b);

    r   = r   + (dt/6.0)*(kr   + 2.0*lr   + 2.0*mr   + nr);
    v   = v   + (dt/6.0)*(kv   + 2.0*lv   + 2.0*mv   + nv);
    q1  = q1  + (dt/6.0)*(kq1  + 2.0*lq1  + 2.0*mq1  + nq1);
    w1b = w1b + (dt/6.0)*(kw1b + 2.0*lw1b + 2.0*mw1b + nw1b);
    q2  = q2  + (dt/6.0)*(kq2  + 2.0*lq2  + 2.0*mq2  + nq2);
    w2b = w2b + (dt/6.0)*(kw2b + 2.0*lw2b + 2.0*mw2b + nw2b);

    state[0] = r[0];
    state[1] = r[1];
    state[2] = r[2];

    state[3] = v[0];
    state[4] = v[1];
    state[5] = v[2];

    state[6] = q1[0];
    state[7] = q1[1];
    state[8] = q1[2];
    state[9] = q1[3];

    state[10] = w1b[0];
    state[11] = w1b[1];
    state[12] = w1b[2];

    state[13] = q2[0];
    state[14] = q2[1];
    state[15] = q2[2];
    state[16] = q2[3];

    state[17] = w2b[0];
    state[18] = w2b[1];
    state[19] = w2b[2];

    return;
}

/* End of integration method. */

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "65803 Didymos", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Hide the mouse initially.

    glfwGetWindowSize(window, &win_width, &win_height);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //Setup ImGui.
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(100.0f,100.0f);
    imstyle.FrameRounding = 5.0f;
    imstyle.WindowRounding = 5.0f;

    //const unsigned char *gpu_vendor = glGetString(GL_VENDOR);

    meshvfn aster1("../obj/vfn/asteroids/didymos/didymain2019.obj");
    meshvfn aster1_axis_x("../obj/vfn/asteroids/didymos/didymain2019_pos_axis_x.obj");
    meshvfn aster1_axis_y("../obj/vfn/asteroids/didymos/didymain2019_pos_axis_y.obj");
    meshvfn aster1_axis_z("../obj/vfn/asteroids/didymos/didymain2019_pos_axis_z.obj");

    meshvfn aster2("../obj/vfn/asteroids/didymos/dimorphos_ellipsoid.obj");
    meshvfn aster2_axis_x("../obj/vfn/asteroids/didymos/dimorphos_ellipsoid_pos_axis_x.obj");
    meshvfn aster2_axis_y("../obj/vfn/asteroids/didymos/dimorphos_ellipsoid_pos_axis_y.obj");
    meshvfn aster2_axis_z("../obj/vfn/asteroids/didymos/dimorphos_ellipsoid_pos_axis_z.obj");

    shader shad("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/dir_light_ad.frag");
    shad.use();

    glm::vec3 light_dir = glm::vec3(0.0f,-1.0f,0.5f);
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 aster_col = glm::vec3(0.5f,0.5f,0.5f);
    glm::vec3 axis_x_col = glm::vec3(1.0f,0.0f,0.0f);
    glm::vec3 axis_y_col = glm::vec3(0.0f,1.0f,0.0f);
    glm::vec3 axis_z_col = glm::vec3(0.0f,0.0f,1.0f);

    shad.set_vec3_uniform("light_dir", light_dir);
    shad.set_vec3_uniform("light_col", light_col);

    G = 6.67430e-20;
    M1 = 5.320591856403073e+11;
    M2 = 4.940814359692687e+09;
    dvec3 semiaxes1 = {0.416194, 0.418765, 0.39309};
    dvec3 semiaxes2 = {0.104, 0.080, 0.066};
    dvec3 r   = {1.19, 0.0, 0.0};
    dvec3 v   = {0.0, 0.00017421523858789, 0.0};
    dvec4 q1  = {1.0, 0.0, 0.0, 0.0};
    dvec3 w1i = {0.0, 0.0, 0.000772269580528465};
    dvec4 q2  = {1.0, 0.0, 0.0, 0.0};
    dvec3 w2i = {0.0, 0.0, 0.000146399360157891};

    q1 = quat2unit(q1);
    q2 = quat2unit(q2);

    duration = 0.0;
    dt = 10.0;
    I1 = ell_inertia(M1, semiaxes1);
    I2 = ell_inertia(M2, semiaxes2);
    dvec3 w1b = iner2body(w1i, quat2mat(q1));
    dvec3 w2b = iner2body(w2i, quat2mat(q2));

    dvec20 state = {  r[0],   r[1],   r[2],
                      v[0],   v[1],   v[2],
                     q1[0],  q1[1],  q1[2], q1[3],
                    w1b[0], w1b[1], w1b[2],
                     q2[0],  q2[1],  q2[2], q2[3],
                    w2b[0], w2b[1], w2b[2]         };
    dvec2 ener0_mom0 = ener_mom(state);

    glm::mat4 projection, view, model;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f,0.05f,0.05f,1.0f);

    double t1 = 0.0f, tnow;
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        tnow = glfwGetTime(); //Elapsed time [sec] since glfwInit().
        time_tick = tnow - t1;
        t1 = tnow;

        event_tick(window); 

        projection = glm::perspective(glm::radians(cam.fov), (float)win_width/win_height, 0.1f,1000.0f);
        cam.move(time_tick);
        view = cam.view();

        shad.set_mat4_uniform("projection", projection);
        shad.set_mat4_uniform("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, (float)(-M2/(M1 + M2))*glm::vec3(state[0], (-M2/(M1 + M2))*state[1], (-M2/(M1 + M2))*state[2]));
        dvec3 rpy1 = quat2ang({state[6], state[7], state[8], state[9]});
        model = glm::rotate(model, (float)rpy1[2], glm::vec3(0.0f,0.0f,1.0f));
        model = glm::rotate(model, (float)rpy1[1], glm::vec3(0.0f,1.0f,0.0f));
        model = glm::rotate(model, (float)rpy1[0], glm::vec3(1.0f,0.0f,0.0f));
        shad.set_mat4_uniform("model", model);
        shad.set_vec3_uniform("mesh_col", aster_col);
        aster1.draw_triangles();

        shad.set_vec3_uniform("mesh_col", axis_x_col);
        aster1_axis_x.draw_triangles();
        shad.set_vec3_uniform("mesh_col", axis_y_col);
        aster1_axis_y.draw_triangles();
        shad.set_vec3_uniform("mesh_col", axis_z_col);
        aster1_axis_z.draw_triangles();




        model = glm::mat4(1.0f);
        model = glm::translate(model, (float)(M1/(M1 + M2))*glm::vec3(state[0], state[1], state[2]));
        dvec3 rpy2 = quat2ang({state[13], state[14], state[15], state[16]});
        model = glm::rotate(model, (float)rpy2[2], glm::vec3(0.0f,0.0f,1.0f));
        model = glm::rotate(model, (float)rpy2[1], glm::vec3(0.0f,1.0f,0.0f));
        model = glm::rotate(model, (float)rpy2[0], glm::vec3(1.0f,0.0f,0.0f));
        shad.set_mat4_uniform("model", model);
        shad.set_vec3_uniform("mesh_col", aster_col);
        aster2.draw_triangles();

        shad.set_vec3_uniform("mesh_col", axis_x_col);
        aster2_axis_x.draw_triangles();
        shad.set_vec3_uniform("mesh_col", axis_y_col);
        aster2_axis_y.draw_triangles();
        shad.set_vec3_uniform("mesh_col", axis_z_col);
        aster2_axis_z.draw_triangles();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
 
        static bool gui_is_closable = true;
        static bool show_energy_conservation = false;
        static bool show_momentum_conservation = false;

        ImGui::SetNextWindowSize(ImVec2(300.0f,500.0f), ImGuiCond_FirstUseEver);
		ImGui::Begin("GUI", &gui_is_closable);
        if (!gui_is_closable)
            glfwSetWindowShouldClose(window, true);
        ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "Right click to toggle the cursor functionality.");
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Text("Duration %.1f [days]", duration);
        ImGui::Checkbox("Show energy conservation", &show_energy_conservation);
        ImGui::End();

        /*
        dvec2 energy_momentum = ener_mom(state);
        energy_data.push_back(fabs( (energy_momentum[0] - ener0_mom0[0])/ener0_mom0[0] ));
        momentum_data.push_back(fabs( (energy_momentum[1] - ener0_mom0[1])/ener0_mom0[1] ));
        time_data.push_back(tnow);
        if (time_data.size() > 1000)
        {
            time_data.erase(time_data.begin());
            energy_data.erase(energy_data.begin());
            momentum_data.erase(energy_data.begin());
        }
        if (show_energy_conservation)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("Energy conservation (real time)", &show_energy_conservation);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("Energy conservation", plot_win_size))
            {
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImPlot::SetupAxes("t [days]", "y");
                ImPlot::SetupAxisLimits(ImAxis_X1, tnow - 70.0, tnow, ImGuiCond_Always); //Automatically scroll the view with time.
                ImPlot::PlotLine("", time_data.data(), energy_data.data(), time_data.size());
                ImPlot::PopStyleColor();
                ImPlot::EndPlot();
            }
            ImGui::End();
        }
        if (show_momentum_conservation)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("Energy conservation (real time)", &show_momentum_conservation);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("Energy conservation", plot_win_size))
            {
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImPlot::SetupAxes("t [days]", "y");
                ImPlot::SetupAxisLimits(ImAxis_X1, tnow - 70.0, tnow, ImGuiCond_Always); //Automatically scroll the view with time.
                ImPlot::PlotLine("", time_data.data(), momentum_data.data(), time_data.size());
                ImPlot::PopStyleColor();
                ImPlot::EndPlot();
            }
            ImGui::End();
        }
        */

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        /*
        char text[100];

        sprintf(text, "time :  %.2f [ days ]", duration);
        ttf.draw(text, 20.0f, win_height - 30.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);

        sprintf(text, "( x, y, z ) :  ( %.2f, %.2f, %.2f ) [ km ]", state[0], state[1], state[2]);
        ttf.draw(text, 20.0f, win_height - 60.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);

        sprintf(text, "( roll 1, pitch 1, yaw 1 ) :  ( %.2f, %.2f, %.2f ) [ deg ]", rpy1[0]*180.0/M_PI, rpy1[1]*180.0/M_PI, rpy1[2]*180.0/M_PI);
        ttf.draw(text, 20.0f, win_height - 90.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);

        sprintf(text, "( quaternion 1 ) :  ( %.2f, %.2f, %.2f, %.2f ) [  ]", state[6], state[7], state[8], state[9]);
        ttf.draw(text, 20.0f, win_height - 120.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);

        sprintf(text, "( roll 2, pitch 2, yaw 2 ) :  ( %.2f, %.2f, %.2f ) [ deg ]", rpy2[0]*180.0/M_PI, rpy2[1]*180.0/M_PI, rpy2[2]*180.0/M_PI);
        ttf.draw(text, 20.0f, win_height - 150.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);

        sprintf(text, "( quaternion 2 ) :  ( %.2f, %.2f, %.2f, %.2f ) [  ]", state[13], state[14], state[15], state[16]);
        ttf.draw(text, 20.0f, win_height - 180.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);

        vec2 energy_momentum = ener_mom(state);
        sprintf(text, "energy error :  %.0e", fabs( (energy_momentum[0] - ener0_mom0[0])/ener0_mom0[0] ));
        ttf.draw(text, 20.0f, win_height - 210.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);
        sprintf(text, "momentum error :  %.0e", fabs( (energy_momentum[1] - ener0_mom0[1])/ener0_mom0[1] ));
        ttf.draw(text, 20.0f, win_height - 230.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);

        sprintf(text, "fps :  %d [ %s ]", (int)(1000.0/ms_per_frame), gpu_vendor);
        ttf.draw(text, 20.0f, win_height - 260.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);


        sprintf(text, "camera : ");
        ttf.draw(text, 20.0f, win_height - 900.0f, win_width, win_height, 0.4f, glm::vec3(0.9f,0.9f,0.0f), text_shad);

        sprintf(text, "( x, y, z ) :  ( %.2f, %.2f, %.2f ) [ km ]", cam.pos[0], cam.pos[1], cam.pos[2]);
        ttf.draw(text, 20.0f, win_height - 930.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);

        sprintf(text, "( roll, pitch, yaw ) :  ( 0.00, %.2f, %.2f ) [ deg ]", cam.pitch, cam.yaw);
        ttf.draw(text, 20.0f, win_height - 960.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);

        sprintf(text, "( fov (zoom) ) :  ( %.2f ) [ deg ]", fov);
        ttf.draw(text, 20.0f, win_height - 990.0f, win_width, win_height, 0.3f, glm::vec3(0.9f,0.9f,0.0f), text_shad);
        */

        glfwSwapBuffers(window);
        glfwPollEvents();

        rk4_step(state);
        duration += dt/86400.0;
    }

    ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
	ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
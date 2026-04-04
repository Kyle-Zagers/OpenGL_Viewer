#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

#pragma once

class Camera {
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -3.0f);
    glm::mat4 view = glm::mat4(1.0f);

    float n = 0.05f;
    float f = 100.0f;
    float r = 0.1f;
    float l = -0.1f;
    float t = 0.1f;
    float b = -0.1f;

    glm::mat4 orthographic = glm::mat4(1.0f);
    glm::mat4 perspective = glm::mat4(1.0f);

    bool perspectiveMode = true;

    glm::mat4 projection = glm::mat4(1.0f);

    void updateProjection() {
        orthographic = glm::transpose(glm::mat4(
            2.0f/(r-l), 0.0f, 0.0f, -(r+l)/(r-l),
            0.0f, 2.0f/(t-b), 0.0f, -(t+b)/(t-b),
            0.0f, 0.0f, 2.0f/(n-f), -(n+f)/(n-f),
            0.0f, 0.0f, 0.0f, 1.0f
        ));

        perspective = glm::transpose(glm::mat4(
            n, 0.0f, 0.0f, 0.0f,
            0.0f, n, 0.0f, 0.0f,
            0.0f, 0.0f, n + f, -f * n,
            0.0f, 0.0f, 1.0f, 0.0f
        ));

        glm::mat4 baseProjection = perspectiveMode ? (orthographic * perspective) : orthographic;
        projection = baseProjection;
    }

    public:
    Camera() {
        updateProjection();
    }
    
    void setFrustum(float left, float right, float bottom, float top, float near, float far) {
        l = left;
        r = right;
        b = bottom;
        t = top;
        n = near;
        f = far;
        updateProjection();
    }

    void setPerspectiveFromFov(float verticalFovDeg, float aspectRatio, float near, float far) {
        float halfHeight = near * std::tan(glm::radians(verticalFovDeg) * 0.5f);
        float halfWidth = halfHeight * aspectRatio;
        setFrustum(-halfWidth, halfWidth, -halfHeight, halfHeight, near, far);
    }

    void makePerspective() {
        perspectiveMode = true;
        updateProjection();
    }

    void makeOrthographic() {
        perspectiveMode = false;
        updateProjection();
    }

    void moveForward(float offset) {
        cameraPos += offset * glm::vec3(view[0][2], view[1][2], view[2][2]);
    }

    void moveBackward(float offset) {
        cameraPos -= offset * glm::vec3(view[0][2], view[1][2], view[2][2]);
    }

    void moveLeft(float offset) {
        cameraPos -= offset * glm::vec3(view[0][0], view[1][0], view[2][0]);
    }

    void moveRight(float offset) {
        cameraPos += offset * glm::vec3(view[0][0], view[1][0], view[2][0]);
    }

    void moveUp(float offset) {
        cameraPos.y += offset;
    }

    void moveDown(float offset) {
        cameraPos.y -= offset;
    }

    void lookDown(float angle) {
        view = glm::rotate(view, glm::radians(-angle), glm::vec3(view[0][0], view[1][0], view[2][0]));
    }

    void lookUp(float angle) {
        view = glm::rotate(view, glm::radians(angle), glm::vec3(view[0][0], view[1][0], view[2][0]));
    }

    void lookRight(float angle) {
        view = glm::rotate(view, glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void lookLeft(float angle) {
        view = glm::rotate(view, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 getPosition() const {
        return cameraPos;
    }

    glm::vec4 clip(glm::mat4 model, glm::vec4 vertex) const {
        return projection * glm::translate(view, -cameraPos) * model * vertex;
    }

    glm::mat4 getViewMatrix() const {
        return glm::translate(view, -cameraPos);
    }

    glm::mat4 getProjectionMatrix() const {
        return projection;
    }

    void refreshProjection() {
        updateProjection();
    }

    float* getNearPtr() {
        return &n;
    }

    float* getFarPtr() {
        return &f;
    }

    float* getRightPtr() {
        return &r;
    }

    float* getLeftPtr() {
        return &l;
    }

    float* getTopPtr() {
        return &t;
    }

    float* getBottomPtr() {
        return &b;
    }

};
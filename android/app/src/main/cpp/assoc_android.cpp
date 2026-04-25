#include <jni.h>

#include <GLES3/gl3.h>
#include <android/log.h>

#include <cmath>

namespace
{
constexpr char kLogTag[] = "assoc-android";
GLuint gProgram = 0u;
GLuint gVbo = 0u;
GLint gTimeUniform = -1;

GLuint compileShader( GLenum type, char const* source )
{
    GLuint shader = glCreateShader( type );
    glShaderSource( shader, 1, &source, nullptr );
    glCompileShader( shader );

    GLint ok = GL_FALSE;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &ok );
    if( ok != GL_TRUE )
    {
        char log[1024] = {};
        glGetShaderInfoLog( shader, sizeof( log ), nullptr, log );
        __android_log_print( ANDROID_LOG_ERROR, kLogTag, "shader compile failed: %s", log );
        glDeleteShader( shader );
        return 0u;
    }
    return shader;
}

GLuint linkProgram()
{
    char const* vertexShader = R"GLSL(
        #version 300 es
        layout(location = 0) in vec2 aPosition;
        layout(location = 1) in vec3 aColor;
        uniform float uTime;
        out vec3 vColor;
        void main()
        {
            float c = cos(uTime);
            float s = sin(uTime);
            mat2 rot = mat2(c, -s, s, c);
            gl_Position = vec4(rot * aPosition, 0.0, 1.0);
            vColor = aColor;
        }
    )GLSL";

    char const* fragmentShader = R"GLSL(
        #version 300 es
        precision mediump float;
        in vec3 vColor;
        out vec4 fragColor;
        void main()
        {
            fragColor = vec4(vColor, 1.0);
        }
    )GLSL";

    GLuint vs = compileShader( GL_VERTEX_SHADER, vertexShader );
    GLuint fs = compileShader( GL_FRAGMENT_SHADER, fragmentShader );
    if( !vs || !fs )
    {
        glDeleteShader( vs );
        glDeleteShader( fs );
        return 0u;
    }

    GLuint program = glCreateProgram();
    glAttachShader( program, vs );
    glAttachShader( program, fs );
    glLinkProgram( program );
    glDeleteShader( vs );
    glDeleteShader( fs );

    GLint ok = GL_FALSE;
    glGetProgramiv( program, GL_LINK_STATUS, &ok );
    if( ok != GL_TRUE )
    {
        char log[1024] = {};
        glGetProgramInfoLog( program, sizeof( log ), nullptr, log );
        __android_log_print( ANDROID_LOG_ERROR, kLogTag, "program link failed: %s", log );
        glDeleteProgram( program );
        return 0u;
    }

    return program;
}
} // namespace

extern "C" JNIEXPORT void JNICALL
Java_hu_assoc_next_MainActivity_nativeInit( JNIEnv*, jclass )
{
    if( gProgram )
    {
        return;
    }

    gProgram = linkProgram();
    gTimeUniform = glGetUniformLocation( gProgram, "uTime" );

    // x, y, r, g, b
    constexpr GLfloat vertices[] = {
        0.0f, 0.65f, 1.0f, 0.85f, 0.10f,
        -0.65f, -0.45f, 0.10f, 0.65f, 1.0f,
        0.65f, -0.45f, 0.95f, 0.20f, 0.35f,
    };

    glGenBuffers( 1, &gVbo );
    glBindBuffer( GL_ARRAY_BUFFER, gVbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
}

extern "C" JNIEXPORT void JNICALL
Java_hu_assoc_next_MainActivity_nativeResize( JNIEnv*, jclass, jint width, jint height )
{
    glViewport( 0, 0, width, height );
}

extern "C" JNIEXPORT void JNICALL
Java_hu_assoc_next_MainActivity_nativeRender( JNIEnv*, jclass, jfloat timeSeconds )
{
    if( !gProgram )
    {
        Java_hu_assoc_next_MainActivity_nativeInit( nullptr, nullptr );
    }

    const float pulse = 0.5f + 0.5f * std::sin( timeSeconds );
    glClearColor( 0.03f, 0.04f + 0.03f * pulse, 0.09f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );

    glUseProgram( gProgram );
    glUniform1f( gTimeUniform, timeSeconds * 0.8f );
    glBindBuffer( GL_ARRAY_BUFFER, gVbo );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), nullptr );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ),
        reinterpret_cast<void*>( 2 * sizeof( GLfloat ) ) );
    glDrawArrays( GL_TRIANGLES, 0, 3 );
}

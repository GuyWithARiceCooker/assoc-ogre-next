package hu.assoc.next;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.SystemClock;
import android.view.Window;
import android.view.WindowManager;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public final class MainActivity extends Activity {
    private AssocSurfaceView surfaceView;

    static {
        System.loadLibrary("assoc_android");
    }

    private static native void nativeInit();

    private static native void nativeResize(int width, int height);

    private static native void nativeRender(float timeSeconds);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(
                WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        surfaceView = new AssocSurfaceView(this);
        setContentView(surfaceView);
    }

    @Override
    protected void onPause() {
        super.onPause();
        surfaceView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        surfaceView.onResume();
    }

    private static final class AssocSurfaceView extends GLSurfaceView {
        AssocSurfaceView(Context context) {
            super(context);
            setEGLContextClientVersion(3);
            setRenderer(new AssocRenderer());
            setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        }
    }

    private static final class AssocRenderer implements GLSurfaceView.Renderer {
        private long startMillis;

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            startMillis = SystemClock.uptimeMillis();
            nativeInit();
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            nativeResize(width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            float timeSeconds = (SystemClock.uptimeMillis() - startMillis) / 1000.0f;
            nativeRender(timeSeconds);
        }
    }
}

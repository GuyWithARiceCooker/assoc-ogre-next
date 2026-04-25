package hu.assoc.next;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public final class MainActivity extends Activity {
    static {
        System.loadLibrary("assoc_android");
    }

    private static native String nativeStatus();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        TextView view = new TextView(this);
        view.setText(nativeStatus());
        view.setTextSize(20.0f);
        view.setPadding(48, 48, 48, 48);
        setContentView(view);
    }
}

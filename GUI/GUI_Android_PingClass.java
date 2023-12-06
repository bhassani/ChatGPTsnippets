
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class PingActivity extends AppCompatActivity {

    private EditText ipAddressEditText;
    private TextView resultTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ping);

        ipAddressEditText = findViewById(R.id.editTextIpAddress);
        resultTextView = findViewById(R.id.textViewResult);

        Button startButton = findViewById(R.id.buttonStart);
        startButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String ipAddress = ipAddressEditText.getText().toString();
                if (!ipAddress.isEmpty()) {
                    new PingTask().execute(ipAddress);
                } else {
                    resultTextView.setText("Please enter an IP address.");
                }
            }
        });
    }

    private class PingTask extends AsyncTask<String, Void, String> {

        @Override
        protected String doInBackground(String... params) {
            String ipAddress = params[0];
            String pingCommand = "ping -c 4 " + ipAddress;

            try {
                Process process = Runtime.getRuntime().exec(pingCommand);
                BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
                StringBuilder output = new StringBuilder();
                String line;

                while ((line = reader.readLine()) != null) {
                    output.append(line).append("\n");
                }

                process.waitFor();
                return output.toString();
            } catch (IOException | InterruptedException e) {
                e.printStackTrace();
                return "Error executing ping command.";
            }
        }

        @Override
        protected void onPostExecute(String result) {
            resultTextView.setText(result);
        }
    }
}


/*
Make sure to add the necessary permissions in the AndroidManifest.xml file to allow the app to access the network.

The layout is defined in res/layout/activity_ping.xml with an EditText for IP address input, a Button for starting the ping, and a TextView for displaying the result.

  <!-- res/layout/activity_ping.xml -->
<?xml version="1.0" encoding="utf-8"?>
<RelativeLayout xmlns:android="http://schemas.android.com/apk/res/android"
    xmlns:tools="http://schemas.android.com/tools"
    android:layout_width="match_parent"
    android:layout_height="match_parent"
    android:padding="16dp"
    tools:context=".PingActivity">

    <EditText
        android:id="@+id/editTextIpAddress"
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:hint="Enter IP Address"/>

    <Button
        android:id="@+id/buttonStart"
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:layout_below="@id/editTextIpAddress"
        android:layout_marginTop="16dp"
        android:text="Start"/>

    <TextView
        android:id="@+id/textViewResult"
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:layout_below="@id/buttonStart"
        android:layout_marginTop="16dp"
        android:gravity="start|top"
        android:text=""
        android:textColor="#000000"
        android:textSize="16sp"/>
</RelativeLayout>
  */



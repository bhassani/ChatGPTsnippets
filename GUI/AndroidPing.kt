import android.os.AsyncTask
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader

class PingActivity : AppCompatActivity() {

    private lateinit var ipAddressEditText: EditText
    private lateinit var resultTextView: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_ping)

        ipAddressEditText = findViewById(R.id.editTextIpAddress)
        resultTextView = findViewById(R.id.textViewResult)

        val startButton: Button = findViewById(R.id.buttonStart)
        startButton.setOnClickListener {
            val ipAddress = ipAddressEditText.text.toString()
            if (ipAddress.isNotEmpty()) {
                PingTask().execute(ipAddress)
            } else {
                resultTextView.text = "Please enter an IP address."
            }
        }
    }

    private inner class PingTask : AsyncTask<String, Void, String>() {

        override fun doInBackground(vararg params: String): String {
            val ipAddress = params[0]
            val pingCommand = "ping -c 4 $ipAddress"

            return try {
                val process = Runtime.getRuntime().exec(pingCommand)
                val reader = BufferedReader(InputStreamReader(process.inputStream))
                val output = StringBuilder()
                var line: String?

                while (reader.readLine().also { line = it } != null) {
                    output.append(line).append("\n")
                }

                process.waitFor()
                output.toString()
            } catch (e: IOException) {
                e.printStackTrace()
                "Error executing ping command."
            } catch (e: InterruptedException) {
                e.printStackTrace()
                "Error executing ping command."
            }
        }

        override fun onPostExecute(result: String) {
            resultTextView.text = result
        }
    }
}

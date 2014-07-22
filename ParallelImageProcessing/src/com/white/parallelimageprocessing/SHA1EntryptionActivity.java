package com.white.parallelimageprocessing;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class SHA1EntryptionActivity extends Activity implements
		Button.OnClickListener {
	private EditText plain_edittext, iter_edittext;
	private TextView key_textview, cipher_textView;
	private Button start_entryption_button, show_result_button;
	private String plain_text;
	private int iterTime;
	private ImageProcessingThread t;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_sha_1_entryption);
		plain_edittext = (EditText) findViewById(R.id.plain_edittext);
		iter_edittext = (EditText) findViewById(R.id.plain_iter_edittext);
		key_textview = (TextView) findViewById(R.id.key_result_textview);
		cipher_textView = (TextView) findViewById(R.id.cipher_result_textview);
		start_entryption_button = (Button) findViewById(R.id.start_encryption);
		start_entryption_button.setOnClickListener(this);
		show_result_button = (Button) findViewById(R.id.show_cipher_button);
		show_result_button.setOnClickListener(this);
		show_result_button.setEnabled(false);

	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.start_encryption:
			plain_text = plain_edittext.getText().toString();
			String iter_string = iter_edittext.getText().toString();
			if ((plain_text.equals("")) || (iter_string.equals(""))) {
				Toast.makeText(this,
						getResources().getString(R.string.input_plain_text),
						Toast.LENGTH_LONG).show();
			} else {
				Toast.makeText(this,
						getResources().getString(R.string.start_computing),
						Toast.LENGTH_LONG).show();
				iterTime = Integer.parseInt(iter_string);
				t = new ImageProcessingThread(this, this.getResources()
						.getStringArray(R.array.command)[0], plain_text,
						iterTime);
				t.start();
				show_result_button.setEnabled(true);
			}
			break;
		case R.id.show_cipher_button:
			String[] results = t.result_string.split("/");// ½á¹û×Ö·û´®
			key_textview.setText("001100000100110101100101");// key
			cipher_textView.setText(results[1]);
			Toast.makeText(this, "SHA1 Hash Result:\nGPU time:"+results[0]+"s CPU time:"+results[2]+"s\nSpeed Up:"+results[3], Toast.LENGTH_LONG).show();
			break;
		default:
			break;
		}

	}
}

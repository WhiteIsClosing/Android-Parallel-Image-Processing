package com.white.parallelimageprocessing;

import android.R.integer;
import android.app.Activity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class MatrixActivity extends Activity {

	private EditText matrix_size_edittext, iter_time_edittext;
	private TextView original_textview, result_textview, original_matrix_title,
			result_matrix_title;
	private OnClickListener listener;
	private Button compute_button, result_button;
	private TextWatcher watcher;
	String original_matrix_string;
	private ImageProcessingThread ipThread;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_matirx);
		matrix_size_edittext = (EditText) findViewById(R.id.matrix_size_edittext);
		iter_time_edittext = (EditText) findViewById(R.id.iter_time_matrix_edittext);
		original_textview = (TextView) findViewById(R.id.original_matrix_textview);
		result_textview = (TextView) findViewById(R.id.result_matrix_textview);
		compute_button = (Button) findViewById(R.id.computing_button);
		result_button = (Button) findViewById(R.id.matrix_result_button);
		original_matrix_title = (TextView) findViewById(R.id.matrix_title_text);
		result_matrix_title = (TextView) findViewById(R.id.matrix_result_text);
		original_matrix_title.setVisibility(View.INVISIBLE);
		result_matrix_title.setVisibility(View.INVISIBLE);
		result_button.setEnabled(false);
		matrix_size_edittext.setText("256");
		iter_time_edittext.setText("10");
		listener = new OnClickListener() {
			@Override
			public void onClick(View v) {
				switch (v.getId()) {
				case R.id.computing_button:
					if ((!compute_button.getText().toString().equals(""))
							&& (!result_button.getText().toString().equals(""))) {
						// 下面调用本地方法
						ipThread = new ImageProcessingThread(
								getApplicationContext(), getApplication()
										.getResources().getStringArray(
												R.array.command)[7],
								Integer.parseInt(iter_time_edittext.getText()
										.toString()),
								Integer.parseInt(matrix_size_edittext.getText()
										.toString()));
						System.out.println("start matrix2");
						ipThread.start();
						result_button.setEnabled(true);
					} else {
						Toast.makeText(
								getApplicationContext(),
								getResources().getString(
										R.string.please_input_iter),
								Toast.LENGTH_LONG).show();
					}
					break;
				case R.id.matrix_result_button:
					result_matrix_title.setVisibility(View.VISIBLE);
					int size=Integer.parseInt(matrix_size_edittext.getText().toString());
					String resultMatrix=size+".................."+size+"\n";
					
					for (int i = 0; i < 4; i++)
						resultMatrix = resultMatrix
								+ "............................\n";
					resultMatrix = resultMatrix
							+ resultMatrix+".................."+resultMatrix;
					result_textview.setText(resultMatrix);
					// 下面顯示結果
					Toast.makeText(getApplicationContext(),
							ipThread.result_string, Toast.LENGTH_LONG).show();
					break;
				default:
					break;
				}
			}
		};
		watcher = new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before,
					int count) {

			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count,
					int after) {

			}

			@Override
			public void afterTextChanged(Editable s) {
				if (!matrix_size_edittext.getText().toString().equals("")) {

					refreshOriginalMatrix();
				} else {
					original_matrix_title.setVisibility(View.INVISIBLE);
				}
			}
		};
		matrix_size_edittext.addTextChangedListener(watcher);
		compute_button.setOnClickListener(listener);
		result_button.setOnClickListener(listener);
		refreshOriginalMatrix();

	}
	void refreshOriginalMatrix(){
		// 文本改变之后的方法
		int size = Integer.parseInt(matrix_size_edittext.getText()
				.toString());

		if (size > 15 && size < 257) {
			original_matrix_title.setVisibility(View.VISIBLE);
			original_matrix_string = "1......................1\n";
			for (int i = 0; i < 4; i++)
				original_matrix_string = original_matrix_string
						+ "............................\n";
			original_matrix_string = original_matrix_string
					
					+ "1......................1";
			original_textview.setText(original_matrix_string);

		} else {
			original_matrix_title.setVisibility(View.INVISIBLE);
			Toast.makeText(
					getApplicationContext(),
					getResources().getString(R.string.size_require),
					Toast.LENGTH_LONG).show();
		}
	}
}

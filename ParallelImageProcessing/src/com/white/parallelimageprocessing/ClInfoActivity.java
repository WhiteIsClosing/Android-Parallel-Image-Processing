package com.white.parallelimageprocessing;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class ClInfoActivity extends Activity {
	private TextView click_textview, info_textview;
	private View.OnClickListener listener;
	ImageProcessingThread get_info;
	String device_info;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_cl_info);
		click_textview = (TextView) findViewById(R.id.click_info);
		info_textview = (TextView) findViewById(R.id.info_textview);
		get_info = new ImageProcessingThread(this, getResources()
				.getStringArray(R.array.command)[8],null, 1);
		get_info.start();
		listener = new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				device_info = get_info.clinfoString;
				String[] split_info = device_info.split("split");
				click_textview.setText(split_info[0]);
				info_textview.setText(split_info[1]);
			}
		};
		click_textview.setOnClickListener(listener);

	}

	@Override
	protected void onStart() {
		super.onStart();
		Toast.makeText(this, getResources().getString(R.string.get_info_text),
				Toast.LENGTH_SHORT).show();
	}
}

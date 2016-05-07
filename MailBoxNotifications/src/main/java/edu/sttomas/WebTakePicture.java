package edu.sttomas;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import javax.servlet.http.HttpServletRequest;

import org.apache.http.HttpResponse;
import org.apache.http.NameValuePair;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.HttpClientBuilder;
import org.apache.http.message.BasicNameValuePair;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

/**
 * @author bmgorski
 *
 */

@Controller
public class WebTakePicture {
	@Autowired
	private MailBoxImageCRUD mailBoxImageCRUD;
	
	private static final String FUNCTION_NAME = "camera";
	
	@RequestMapping(method = RequestMethod.GET, value = "/user/overview")
	public String provideUploadInfo(Model model) {
		model.addAttribute("images", mailBoxImageCRUD.findFirst20ByDeviceIdOrderByTimestampDesc(MailBoxNotificationsApplication.DEVICE_ID));		
		return "overView";
	}

	@RequestMapping(method = RequestMethod.POST, value = "/user/overview")
	public String handleFileUpload(HttpServletRequest request) throws ClientProtocolException, IOException {

		String url = "https://api.particle.io/v1/devices/" + MailBoxNotificationsApplication.DEVICE_ID + "/" + FUNCTION_NAME;

		HttpClient client = HttpClientBuilder.create().build();
		HttpPost postCloud = new HttpPost(url);

		// add request header
		postCloud.addHeader("User-Agent", request.getHeader("User-Agent"));
		
		List<NameValuePair> urlParameters = new ArrayList<NameValuePair>();
		urlParameters.add(new BasicNameValuePair("access_token", MailBoxNotificationsApplication.ACCESS_TOKEN));
		urlParameters.add(new BasicNameValuePair("arg", "takePics"));

		postCloud.setEntity(new UrlEncodedFormEntity(urlParameters));
		
		
		HttpResponse response = client.execute(postCloud);

		BufferedReader rd = new BufferedReader(new InputStreamReader(response.getEntity().getContent()));

		StringBuffer result = new StringBuffer();
		String line = "";
		while ((line = rd.readLine()) != null) {
			result.append(line);
		}
		
		return "redirect:overview";
	}
}

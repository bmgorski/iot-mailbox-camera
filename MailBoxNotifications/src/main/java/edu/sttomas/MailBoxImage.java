package edu.sttomas;

import java.sql.Timestamp;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;

import org.springframework.format.annotation.DateTimeFormat;

@Entity
public class MailBoxImage {

	@Id
	private String id;
	
	@Column(name="user_id")
	private String userId;
	
	@Column(name="device_id")
	private String deviceId;
	
	@Column(name="device_state")
	private int deviceState;
	
	@Column(name="image_name")
	private String imageName;
	
	@DateTimeFormat (pattern="dd-MMM-YYYY")
	private Timestamp timestamp;

	protected MailBoxImage() {
	}

	public MailBoxImage(String id, String user_id, String device_id, int device_state, String image_name,
			Timestamp timestamp) {
		this.id = id;
		this.userId = user_id;
		this.deviceId = device_id;
		this.deviceState = device_state;
		this.imageName = image_name;
		this.timestamp = timestamp;
	}

	public String getId() {
		return id;
	}

	public String getUserId() {
		return userId;
	}

	public String getDeviceId() {
		return deviceId;
	}

	public int getDeviceState() {
		return deviceState;
	}

	public String getImageName() {
		return imageName;
	}

	public Timestamp getTimestamp() {
		return timestamp;
	}
}

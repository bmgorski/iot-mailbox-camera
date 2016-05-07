package edu.sttomas;

import java.util.List;

import org.springframework.data.repository.CrudRepository;

public interface MailBoxImageCRUD extends CrudRepository<MailBoxImage, String> {
	public List<MailBoxImage> findFirst20ByDeviceIdOrderByTimestampDesc(String device_id);
}

# Contributing Guide — Protocol Module

Cảm ơn bạn đã quan tâm đóng góp cho **Adaptive Protocol**.  
Module này là phần lõi kết nối giữa các hạ tầng thích nghi (Adaptive OS, IBCS, DIP, AIForDriver).  

---

## 1. Chuẩn bị môi trường
- **Yêu cầu:** `C++23`, `Lua 5.4.6`, `CMake >= 3.20`, Git.  
- Clone repo kèm submodules:
  ```bash
  git clone --recurse-submodules https://github.com/AdaptiveIntelligenceCircle/AdaptiveProtocol.git

## 2. Quy tắc code

Dùng snake_case cho file và hàm.

Comment rõ ràng, tránh “magic number”.

Các module phải tương thích cross-platform (Linux/Windows).

Luôn cập nhật CHANGELOG.md khi có thay đổi lớn.

Không commit key/token hoặc dữ liệu riêng tư.

## 3. Quy trình đóng góp

1. Fork repo 
2. Create new branch 
``` bash 
git checkout -b feature/<tên-tính-năng>
```

3. Commit
``` bash 
feat: thêm logic sync handshake
fix: sửa lỗi parse message_broker
refactor: tách lớp encryption_layer
``` 

4. Open pull request -> wait for the review from maintainer. 

## 4. Testing 

```bash 
cd test/
bash run_protocol_test.sh
```

## 5. Liên hệ hỗ trợ

Vấn đề kỹ thuật: ndtribk@gmail.com

Vấn đề bảo mật: ndtribk@gmail.com


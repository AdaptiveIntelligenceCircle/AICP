## 🔐 `SECURITY.md`
> Nhấn mạnh triết lý **Trust-based Self-Defense** của Adaptive Protocol, tránh lộ thông tin handshake và token.

```markdown
# Security Policy — Adaptive Protocol

## 1. Triết lý bảo mật
Adaptive Protocol được thiết kế dựa trên nguyên lý **Trust-based Self-Defense** —  
nghĩa là hệ thống không chỉ phát hiện tấn công, mà còn tự thích nghi và phản ứng ngược.

---

## 2. Báo cáo lỗ hổng
Nếu bạn phát hiện:
- Rò rỉ key/token,
- Lỗi handshake / message broker,
- Lỗ hổng Lua script injection,
- Hoặc bất kỳ vấn đề liên quan đến sync / encryption,

Hãy gửi chi tiết qua:
📧 `ndtribk@gmail.com`  
hoặc dùng form an toàn tại: [https://adaptivecircle.org/security-report](#)

> **Không công khai lỗ hổng** trước khi được xác nhận và vá bởi nhóm bảo mật.

---

## 3. Hướng xử lý sự cố
- Bản vá sẽ được phát hành qua branch `security/patch-*`.  
- Mọi pull request liên quan bảo mật phải đi kèm file `SECURITY_PATCH_NOTE.md`.  
- Trong trường hợp nghiêm trọng, repo có thể bị tạm khoá public tạm thời.

---

## 4. Phạm vi bảo mật
Module này bao gồm:
- `core/handshake.cpp`, `sync_manager.cpp`
- `security/` (auth_manager, self_defense_guard, cert_manager)
- `lua/` scripts có quyền ảnh hưởng tới runtime.

Các phần khác (ví dụ logging, test mock) **không nằm trong phạm vi nhạy cảm**.

---

## 5. Tri ân
Chúng tôi cảm ơn mọi người đã giúp giữ an toàn cho Adaptive Intelligence Circle.

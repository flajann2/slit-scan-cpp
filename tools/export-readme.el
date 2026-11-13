;;; export-readme.el --- Export README.org to .github/README.md  -*- lexical-binding: t; -*-

;; Run from the repository root:
;;   emacs --batch -l tools/export-readme.el
;;
;; GitHub shows .github/README.md in preference to README.org.  Links in
;; that file resolve from .github/, so this rewrites relative links to
;; root-relative ones (docs/x.png -> /docs/x.png).  ox-md also turns links
;; to .org files into .md; those are turned back when no .md exists.

(require 'package)
(package-initialize)
(require 'ox-gfm)

;; Show .webp (and the usual formats) inline rather than as plain links.
(push (cons "file" "\\.\\(jpe?g\\|png\\|gif\\|svg\\|webp\\|avif\\)\\'")
      org-html-inline-image-rules)

(defun export-readme--root-relative (text backend _info)
  "Make relative Markdown links in TEXT work from .github/README.md."
  (if (not (org-export-derived-backend-p backend 'md))
      text
    (replace-regexp-in-string
     "](\\([^)/#<][^):]*\\))"
     (lambda (m)
       (let* ((path (match-string 1 m))
              (org (concat (file-name-sans-extension path) ".org")))
         (when (and (string-suffix-p ".md" path)
                    (not (file-exists-p path))
                    (file-exists-p org))
           (setq path org))
         (concat "](/" path ")")))
     text t t)))

(add-to-list 'org-export-filter-final-output-functions
             #'export-readme--root-relative)

(let ((org-export-with-broken-links nil))
  (with-current-buffer (find-file-noselect "README.org")
    (message "Wrote %s" (org-gfm-export-to-markdown))))

;;; export-readme.el ends here

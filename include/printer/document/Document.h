/*
 * Document.h
 *
 *  Created on: 21 juin 2012
 *      Author: pwilke
 */

#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <vector>
#include <string>

class Document {
public:
	Document() {
	}
	virtual ~Document() {
	}
};

class BreakPoint: public Document {
public:
	BreakPoint() {
	}
	virtual ~BreakPoint() {
	}
};

class StringDocument: public Document {
public:
	StringDocument() {
	}
	virtual ~StringDocument() {
	}
	StringDocument(std::string s) :
			stringDocument(s) {
	}

	std::string getString() {
		return stringDocument;
	}
	void setString(std::string s) {
		stringDocument = s;
	}
private:
	std::string stringDocument;
};

class DocumentList: public Document {
public:
	DocumentList() {
	}
	virtual ~DocumentList(){

	}
	DocumentList(std::string _beginToken = "", std::string _separator = "",
			std::string _endToken = "", bool _alignment = true);
	void addDocumentToList(Document* d) {
		docs.push_back(d);
	}
	void addStringToList(std::string s) {
		addDocumentToList(new StringDocument(s));
	}
	void addBreakPoint() {
		addDocumentToList(new BreakPoint());
	}
	std::vector<Document*> getDocs() {
		return docs;
	}
	void setList(std::vector<Document*> ld) {
		docs = ld;
	}
	std::string getBeginToken() {
		return beginToken;
	}
	std::string getEndToken() {
		return endToken;
	}
	std::string getSeparator() {
		return separator;
	}
	bool getAlignment() {
		return alignment;
	}

private:
	std::vector<Document*> docs;
	std::string beginToken;
	std::string separator;
	std::string endToken;
	bool alignment;
};



#endif /* DOCUMENTLIST_H_ */

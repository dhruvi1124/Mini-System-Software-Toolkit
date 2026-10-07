import os
import docx
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import parse_xml
from docx.oxml.ns import nsdecls

def create_pbl_report():
    doc = docx.Document()

    # Colors (Navy Academic Theme)
    COLOR_PRIMARY = RGBColor(0, 51, 102)     # Deep Navy (#003366)
    COLOR_SECONDARY = RGBColor(26, 82, 118)  # Slate Blue (#1A5276)
    COLOR_DARK = RGBColor(46, 64, 83)        # Dark Charcoal (#2E4053)
    COLOR_MUTED = RGBColor(100, 110, 120)    # Muted Gray
    COLOR_BLACK = RGBColor(0, 0, 0)

    HEX_PRIMARY = "003366"
    HEX_LIGHT_BG = "F2F4F4"
    HEX_CODE_BG = "F8F9F9"
    HEX_BORDER = "CCCCCC"

    # Margins (A4: 1 inch)
    for section in doc.sections:
        section.top_margin = Inches(1)
        section.bottom_margin = Inches(1)
        section.left_margin = Inches(1)
        section.right_margin = Inches(1)

    # Base Font: Times New Roman
    normal_style = doc.styles['Normal']
    normal_style.font.name = 'Times New Roman'
    normal_style.font.size = Pt(11)
    normal_style.font.color.rgb = COLOR_BLACK
    normal_style.paragraph_format.line_spacing = 1.15
    normal_style.paragraph_format.space_after = Pt(6)
    normal_style.paragraph_format.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY

    def add_title_line(text, font_size=22, bold=True, color=COLOR_PRIMARY, align=WD_ALIGN_PARAGRAPH.CENTER, space_after=12):
        p = doc.add_paragraph()
        p.alignment = align
        p.paragraph_format.space_after = Pt(space_after)
        p.paragraph_format.line_spacing = 1.15
        run = p.add_run(text)
        run.font.size = Pt(font_size)
        run.font.bold = bold
        run.font.color.rgb = color
        run.font.name = 'Times New Roman'
        return p

    def add_h1(text):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.LEFT
        p.paragraph_format.space_before = Pt(18)
        p.paragraph_format.space_after = Pt(8)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.font.size = Pt(16)
        run.font.bold = True
        run.font.color.rgb = COLOR_PRIMARY
        run.font.name = 'Times New Roman'
        return p

    def add_h2(text):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.LEFT
        p.paragraph_format.space_before = Pt(14)
        p.paragraph_format.space_after = Pt(6)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.font.size = Pt(13)
        run.font.bold = True
        run.font.color.rgb = COLOR_SECONDARY
        run.font.name = 'Times New Roman'
        return p

    def add_h3(text):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.LEFT
        p.paragraph_format.space_before = Pt(10)
        p.paragraph_format.space_after = Pt(4)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.font.size = Pt(11.5)
        run.font.bold = True
        run.font.color.rgb = COLOR_DARK
        run.font.name = 'Times New Roman'
        return p

    def add_p(text, bold_prefix=None, space_after=6):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
        p.paragraph_format.space_after = Pt(space_after)
        p.paragraph_format.line_spacing = 1.15
        if bold_prefix:
            run_b = p.add_run(bold_prefix)
            run_b.font.bold = True
            run_b.font.color.rgb = COLOR_DARK
            run_b.font.name = 'Times New Roman'
        run_t = p.add_run(text)
        run_t.font.name = 'Times New Roman'
        return p

    def add_bullet(text, bold_prefix=None):
        p = doc.add_paragraph(style='List Bullet')
        p.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
        p.paragraph_format.space_after = Pt(4)
        p.paragraph_format.line_spacing = 1.15
        if bold_prefix:
            run_b = p.add_run(bold_prefix)
            run_b.font.bold = True
            run_b.font.color.rgb = COLOR_DARK
            run_b.font.name = 'Times New Roman'
        run_t = p.add_run(text)
        run_t.font.name = 'Times New Roman'
        return p

    def add_placeholder_box(caption, short_explanation=None):
        table = doc.add_table(rows=1, cols=1)
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        cell = table.cell(0, 0)

        shading = parse_xml(f'<w:shd {nsdecls("w")} w:fill="FAFAFA"/>')
        cell._tc.get_or_add_tcPr().append(shading)

        borders = parse_xml(f'''
            <w:tcBorders {nsdecls("w")}>
                <w:top w:val="single" w:sz="6" w:space="0" w:color="{HEX_BORDER}"/>
                <w:left w:val="single" w:sz="6" w:space="0" w:color="{HEX_BORDER}"/>
                <w:bottom w:val="single" w:sz="6" w:space="0" w:color="{HEX_BORDER}"/>
                <w:right w:val="single" w:sz="6" w:space="0" w:color="{HEX_BORDER}"/>
            </w:tcBorders>
        ''')
        cell._tc.get_or_add_tcPr().append(borders)

        p = cell.paragraphs[0]
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p.paragraph_format.space_before = Pt(28)
        p.paragraph_format.space_after = Pt(28)
        run = p.add_run("[INSERT SCREENSHOT HERE]")
        run.font.bold = True
        run.font.size = Pt(11)
        run.font.color.rgb = COLOR_MUTED
        run.font.name = 'Times New Roman'

        p_cap = doc.add_paragraph()
        p_cap.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p_cap.paragraph_format.space_before = Pt(6)
        p_cap.paragraph_format.space_after = Pt(4)
        p_cap.paragraph_format.keep_with_next = True
        run_cap = p_cap.add_run(caption)
        run_cap.font.bold = True
        run_cap.font.size = Pt(9.5)
        run_cap.font.color.rgb = COLOR_SECONDARY
        run_cap.font.name = 'Times New Roman'

        if short_explanation:
            add_p(short_explanation, space_after=10)

    def add_code_snippet(code_text, caption):
        p_cap = doc.add_paragraph()
        p_cap.alignment = WD_ALIGN_PARAGRAPH.LEFT
        p_cap.paragraph_format.space_before = Pt(10)
        p_cap.paragraph_format.space_after = Pt(3)
        p_cap.paragraph_format.keep_with_next = True
        run_cap = p_cap.add_run(caption)
        run_cap.font.bold = True
        run_cap.font.size = Pt(10)
        run_cap.font.color.rgb = COLOR_SECONDARY
        run_cap.font.name = 'Times New Roman'

        table = doc.add_table(rows=1, cols=1)
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        cell = table.cell(0, 0)

        shading = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{HEX_CODE_BG}"/>')
        cell._tc.get_or_add_tcPr().append(shading)

        borders = parse_xml(f'''
            <w:tcBorders {nsdecls("w")}>
                <w:top w:val="single" w:sz="4" w:space="0" w:color="{HEX_PRIMARY}"/>
                <w:left w:val="single" w:sz="18" w:space="0" w:color="{HEX_PRIMARY}"/>
                <w:bottom w:val="single" w:sz="4" w:space="0" w:color="{HEX_PRIMARY}"/>
                <w:right w:val="single" w:sz="4" w:space="0" w:color="{HEX_PRIMARY}"/>
            </w:tcBorders>
        ''')
        cell._tc.get_or_add_tcPr().append(borders)

        p = cell.paragraphs[0]
        p.alignment = WD_ALIGN_PARAGRAPH.LEFT
        p.paragraph_format.space_after = Pt(2)
        p.paragraph_format.space_before = Pt(2)
        p.paragraph_format.line_spacing = 1.0
        run = p.add_run(code_text)
        run.font.name = 'Consolas'
        run.font.size = Pt(9.5)
        run.font.color.rgb = RGBColor(30, 30, 30)

        p_spacer = doc.add_paragraph()
        p_spacer.paragraph_format.space_after = Pt(6)

    def style_table(table):
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        hdr_cells = table.rows[0].cells
        for cell in hdr_cells:
            shading = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{HEX_PRIMARY}"/>')
            cell._tc.get_or_add_tcPr().append(shading)
            for p in cell.paragraphs:
                p.alignment = WD_ALIGN_PARAGRAPH.LEFT
                for run in p.runs:
                    run.font.bold = True
                    run.font.color.rgb = RGBColor(255, 255, 255)
                    run.font.size = Pt(10)
                    run.font.name = 'Times New Roman'

        for row_idx, row in enumerate(table.rows[1:], start=1):
            bg_color = HEX_LIGHT_BG if row_idx % 2 == 1 else "FFFFFF"
            for cell in row.cells:
                shading = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{bg_color}"/>')
                cell._tc.get_or_add_tcPr().append(shading)
                borders = parse_xml(f'''
                    <w:tcBorders {nsdecls("w")}>
                        <w:top w:val="single" w:sz="4" w:space="0" w:color="{HEX_BORDER}"/>
                        <w:left w:val="single" w:sz="4" w:space="0" w:color="{HEX_BORDER}"/>
                        <w:bottom w:val="single" w:sz="4" w:space="0" w:color="{HEX_BORDER}"/>
                        <w:right w:val="single" w:sz="4" w:space="0" w:color="{HEX_BORDER}"/>
                    </w:tcBorders>
                ''')
                cell._tc.get_or_add_tcPr().append(borders)
                for p in cell.paragraphs:
                    for run in p.runs:
                        run.font.size = Pt(9.5)
                        run.font.name = 'Times New Roman'

    # 1. TITLE PAGE
    add_title_line("VISHWAKARMA GOVERNMENT ENGINEERING COLLEGE", font_size=18, bold=True, space_after=4)
    add_title_line("CHANDKHEDA, AHMEDABAD", font_size=14, bold=True, color=COLOR_SECONDARY, space_after=18)

    add_title_line("DEPARTMENT OF COMPUTER ENGINEERING", font_size=13, bold=True, color=COLOR_DARK, space_after=30)

    add_title_line("PROJECT-BASED LEARNING (PBL) REPORT", font_size=15, bold=True, color=COLOR_MUTED, space_after=10)

    add_title_line("MINI SYSTEM SOFTWARE TOOLKIT", font_size=24, bold=True, color=COLOR_PRIMARY, space_after=6)
    add_title_line("A Menu-Driven C11 Framework Simulating Program Translation and Execution", font_size=12, bold=False, color=COLOR_SECONDARY, space_after=36)

    p_meta = doc.add_paragraph()
    p_meta.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_meta.paragraph_format.line_spacing = 1.3
    
    r = p_meta.add_run("Activity: ")
    r.bold = True
    p_meta.add_run("Application / Software Development\n")

    r = p_meta.add_run("Subject: ")
    r.bold = True
    p_meta.add_run("System Software\n")
    
    r = p_meta.add_run("Subject Code: ")
    r.bold = True
    p_meta.add_run("BE05000261\n")

    r = p_meta.add_run("Duration: ")
    r.bold = True
    p_meta.add_run("15 Hours | ")

    r = p_meta.add_run("Submission Date: ")
    r.bold = True
    p_meta.add_run("03-10-2026\n\n")

    p_table = doc.add_table(rows=1, cols=2)
    p_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    cell_lh = p_table.cell(0, 0)
    cell_rh = p_table.cell(0, 1)

    p_lh = cell_lh.paragraphs[0]
    p_lh.add_run("Prepared By:\n").bold = True
    p_lh.add_run("[Student Name 1] (Enrollment No: [XXXXXXXXXXXX])\n")
    p_lh.add_run("[Student Name 2] (Enrollment No: [XXXXXXXXXXXX])\n")
    p_lh.add_run("[Student Name 3] (Enrollment No: [XXXXXXXXXXXX])\n")

    p_rh = cell_rh.paragraphs[0]
    p_rh.add_run("Submitted To:\n").bold = True
    p_rh.add_run("[Faculty Supervisor Name]\n")
    p_rh.add_run("Department of Computer Engineering\n")
    p_rh.add_run("VGEC Chandkheda\n")

    doc.add_page_break()

    # 2. CERTIFICATE
    add_h1("1. CERTIFICATE")
    add_p("This is to certify that the project entitled \"MINI SYSTEM SOFTWARE TOOLKIT\" submitted by [Student Name(s)] bearing Enrollment Number(s) [XXXXXXXXXXXX] to the Department of Computer Engineering, Vishwakarma Government Engineering College, Chandkheda, is a bonafide record of Project-Based Learning (PBL) work carried out under guidance for the course System Software (BE05000261).")
    add_p("The project satisfies the academic requirements and standards prescribed for the Bachelor of Engineering degree program.")

    doc.add_paragraph().paragraph_format.space_after = Pt(30)

    sig_table = doc.add_table(rows=1, cols=2)
    sig_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    c1 = sig_table.cell(0, 0).paragraphs[0]
    c1.add_run("_________________________\nInternal Guide / Faculty\nDepartment of Computer Engg.\nVGEC Chandkheda")
    c2 = sig_table.cell(0, 1).paragraphs[0]
    c2.add_run("_________________________\nHead of Department (HOD)\nDepartment of Computer Engg.\nVGEC Chandkheda")

    # 3. DECLARATION
    add_h1("2. DECLARATION")
    add_p("We hereby declare that the work presented in this PBL report entitled \"Mini System Software Toolkit\" is an original outcome of our software engineering efforts carried out at Vishwakarma Government Engineering College, Chandkheda. We confirm that the implementation is based on authentic C11 modular software development and has not been submitted elsewhere for any degree or diploma award.")

    add_p("Place: Chandkheda, Ahmedabad\nDate: 03-10-2026", bold_prefix="Submission Details: ")

    # 4. ACKNOWLEDGEMENT
    add_h1("3. ACKNOWLEDGEMENT")
    add_p("We express our profound gratitude to Vishwakarma Government Engineering College, Chandkheda, and the Department of Computer Engineering for providing the requisite academic facilities and technical infrastructure to execute this project.")
    add_p("We extend our heartfelt thanks to our faculty guide and course instructor for their invaluable guidance, constructive feedback, and continuous support throughout the duration of the System Software PBL course. Finally, we thank our peers for their collaborative insights during the software testing phase.")

    doc.add_page_break()

    # 5. ABSTRACT
    add_h1("4. ABSTRACT")
    add_p("System software forms the fundamental infrastructural backbone of modern computing systems, serving as the bridge between application software, compiler toolchains, and hardware execution platforms. Despite its critical importance, students frequently struggle to reconcile theoretical system software concepts—such as lexical tokenization, table-driven symbol resolution, multi-pass assembly translation, macro expansion, top-down predictive parsing, intermediate code representation, link-time memory relocation, and machine-independent code optimization—with their actual programmatic mechanics.")

    add_p("The Mini System Software Toolkit addresses this educational gap by offering a fully functional, highly modular, menu-driven academic simulation framework implemented entirely in ISO C11. Operating over standard input files and producing deterministic output text artifacts, the toolkit integrates eight distinct core system software modules into a unified executable architecture:")

    add_bullet("Scans raw source text into categorized C17 lexical tokens.", bold_prefix="Lexical Analyzer: ")
    add_bullet("Manages identifier memory attributes, data types, scope, and addresses starting at 1000.", bold_prefix="Symbol Table: ")
    add_bullet("Performs address allocation (Pass 1) and machine opcode encoding (Pass 2).", bold_prefix="Two Pass Assembler: ")
    add_bullet("Processes MNT/MDT tables, formal-to-actual argument binding, and macro body expansion.", bold_prefix="Macro Processor: ")
    add_bullet("Validates arithmetic grammar using recursive descent predictive parsing.", bold_prefix="Recursive Descent Parser: ")
    add_bullet("Translates infix arithmetic expressions into 3-address quadruple records.", bold_prefix="Quadruple Generator: ")
    add_bullet("Resolves external module symbols, calculates relocation offsets, and emits a linked memory map.", bold_prefix="Linker Loader: ")
    add_bullet("Applies constant folding, constant propagation, algebraic simplification, and dead code elimination.", bold_prefix="Code Optimizer: ")

    add_p("Through clean software decoupling, file-based input/output transparency, and detailed execution logging, this project provides undergraduate computer engineering students with an invaluable hands-on laboratory platform for mastering system software internal engineering.")

    doc.add_page_break()

    # 6. TABLE OF CONTENTS
    add_h1("5. TABLE OF CONTENTS")
    toc_items = [
        ("1. Certificate", "2"),
        ("2. Declaration", "2"),
        ("3. Acknowledgement", "2"),
        ("4. Abstract", "3"),
        ("5. Table of Contents & Document Lists", "4"),
        ("6. Introduction", "5"),
        ("7. Problem Statement", "6"),
        ("8. Objectives", "7"),
        ("9. Scope of the Project", "7"),
        ("10. System Requirements", "8"),
        ("11. System Architecture", "9"),
        ("12. Project Structure", "10"),
        ("13. Detailed Module Implementation", "11"),
        ("    13.1 Lexical Analyzer", "11"),
        ("    13.2 Symbol Table", "12"),
        ("    13.3 Two Pass Assembler", "13"),
        ("    13.4 Macro Processor", "14"),
        ("    13.5 Recursive Descent Parser", "15"),
        ("    13.6 Quadruple Generator", "16"),
        ("    13.7 Linker Loader", "17"),
        ("    13.8 Code Optimizer", "18"),
        ("14. Algorithms / Working Methodology", "19"),
        ("15. Input and Output Description", "20"),
        ("16. Testing and Results", "21"),
        ("17. Advantages", "22"),
        ("18. Limitations", "22"),
        ("19. Future Scope", "22"),
        ("20. Learning Outcomes", "23"),
        ("21. Conclusion", "23"),
        ("22. References", "24"),
        ("23. Appendix", "24")
    ]
    
    toc_table = doc.add_table(rows=len(toc_items)+1, cols=2)
    toc_table.rows[0].cells[0].paragraphs[0].add_run("Section Title").bold = True
    toc_table.rows[0].cells[1].paragraphs[0].add_run("Page No.").bold = True
    for i, (title, pg) in enumerate(toc_items):
        toc_table.rows[i+1].cells[0].paragraphs[0].add_run(title)
        toc_table.rows[i+1].cells[1].paragraphs[0].add_run(pg)
    style_table(toc_table)

    add_h2("List of Figures")
    fig_items = [
        ("Figure 11.1: Overall Architectural Block Diagram of Mini System Software Toolkit", "9"),
        ("Figure 13.1: Main Toolkit Menu Execution Output", "11"),
        ("Figure 13.2: Lexical Analyzer Execution and Generated Token Output", "11"),
        ("Figure 13.3: Symbol Table Operations and Result Output", "12"),
        ("Figure 13.4a: Two Pass Assembler Assembly Source Input", "13"),
        ("Figure 13.4b: Two Pass Assembler Intermediate Code, Symbol Table, and Object Code Output", "13"),
        ("Figure 13.5: Macro Processor Definition and Expanded Output", "14"),
        ("Figure 13.6: Recursive Descent Parser Valid and Invalid Expression Result", "15"),
        ("Figure 13.7: Quadruple Generator 3-Address Intermediate Code Output", "16"),
        ("Figure 13.8: Linker Loader External Symbol Table and Linked Memory Map Output", "17"),
        ("Figure 13.9: Code Optimizer Input and Optimized Code Output", "18"),
        ("Figure 14.1: Master Control Flowchart Suite for All Toolkit Modules", "19")
    ]
    for fig_title, pg in fig_items:
        add_bullet(f"{fig_title} ............................................................................................................ Page {pg}")

    add_h2("List of Tables")
    tab_items = [
        ("Table 10.1: Minimum Hardware & Software Requirements Specification", "8"),
        ("Table 12.1: Complete Project Folder and Subdirectory Layout", "10"),
        ("Table 13.1: Assembler Instruction Set & Machine Opcode Table", "13"),
        ("Table 15.1: Modules Input and Output Specification Matrix", "20"),
        ("Table 16.1: Master Test Suite Results & System Validation Matrix", "21")
    ]
    for tab_title, pg in tab_items:
        add_bullet(f"{tab_title} ............................................................................................................ Page {pg}")

    doc.add_page_break()

    # 6. INTRODUCTION
    add_h1("6. INTRODUCTION")
    add_p("System software consists of low-level computer programs that manage hardware resources and provide essential infrastructure services required for user application programs and software toolchains. Unlike application software, which focuses on domain-specific tasks such as document processing or web browsing, system software manages execution memory, hardware interfaces, instruction compilation, program linkage, and execution environments.")

    add_p("In modern computing systems, program translation and execution rely on a chain of system software components that transform high-level human-readable source code into binary machine instructions executed directly by the Central Processing Unit (CPU). Understanding these components requires analyzing eight essential phases of language processing and program loading:")

    add_bullet("The initial compiler phase that reads raw source characters, strips comments and whitespace, and groups lexemes into meaningful lexical tokens (keywords, identifiers, literals, operators).", bold_prefix="1. Lexical Analysis: ")
    add_bullet("A central data structure created during compilation and assembly that stores information about program symbols, including data types, memory addresses starting from 1000, scopes, and storage allocation.", bold_prefix="2. Symbol Table Management: ")
    add_bullet("A fundamental translator that converts assembly language mnemonics into machine code. A two-pass architecture separates symbol address assignment (Pass 1) from machine code generation (Pass 2).", bold_prefix="3. Assembly Translation: ")
    add_bullet("A specialized pre-compiler component that expands macro definitions, maintains Macro Name Tables (MNT) and Macro Definition Tables (MDT), and performs formal-to-actual argument substitution.", bold_prefix="4. Macro Expansion: ")
    add_bullet("A syntax analysis technique that checks whether a token sequence satisfies a formal context-free grammar using a hierarchy of mutually recursive functions.", bold_prefix="5. Recursive Descent Parsing: ")
    add_bullet("An intermediate code generation mechanism that converts complex nested expressions into standardized 4-tuple records (Operator, Argument 1, Argument 2, Result Target).", bold_prefix="6. Intermediate Code Generation (Quadruples): ")
    add_bullet("System software that combines independently compiled object modules into a single executable layout, resolves external cross-references, and calculates memory relocation offsets.", bold_prefix="7. Linking and Loading: ")
    add_bullet("Code transformation passes that analyze intermediate code to reduce execution latency, eliminate redundant code, and reduce memory footprint without altering program semantics.", bold_prefix="8. Code Optimization: ")

    add_p("The Mini System Software Toolkit implements a clean, educational simulation of all eight core components in ISO C11, establishing a robust hands-on software foundation for computer engineering students.")

    # 7. PROBLEM STATEMENT
    add_h1("7. PROBLEM STATEMENT")
    add_p("In standard undergraduate Computer Engineering curricula, System Software and Compiler Design are frequently taught as theoretical subjects dominated by formal proofs, state machine diagrams, and abstract grammar rules. Students learn the mathematical concepts behind finite automata, LL(1) parsing tables, relocation registers, and constant folding algorithms, but rarely gain practical experience implementing these concepts in executable code.")

    add_p("Existing commercial toolchains—such as GCC, LLVM, GNU ld, and NASM—are immensely complex production systems containing millions of lines of C/C++ code. Their intricate build systems, platform-specific abstractions, and performance optimizations make it difficult for students to isolate and understand basic internal algorithms.")

    add_p("Furthermore, isolated academic assignments often require students to code individual components (such as a standalone parser or symbol table) without demonstrating how these components interact across program translation and loading. This leads to several learning challenges:")

    add_bullet("Students struggle to conceptualize how data structures flow between compilation and loading phases.", bold_prefix="Fragmented Understanding: ")
    add_bullet("Theoretical textbooks rarely detail practical file parsing, edge case error handling, or memory management in system software.", bold_prefix="Lack of Implementation Context: ")
    add_bullet("Without clean visual output artifacts, students cannot trace how source code transforms into tokens, quadruples, object code, and linked memory maps.", bold_prefix="Opaque Translation Mechanics: ")

    add_p("To overcome these challenges, there is a clear academic need for a unified, modular, menu-driven System Software Toolkit written in standard C11. This toolkit must implement the eight fundamental language translation and program loading phases, operate deterministically over transparent text input/output files, and provide an accessible educational platform for practical exploration.")

    # 8. OBJECTIVES
    add_h1("8. OBJECTIVES")
    add_p("The overarching goal of the Mini System Software Toolkit project is to design, implement, and validate a unified educational system software simulation environment in ISO C11. The project satisfies eight specific technical and pedagogical objectives:")

    add_bullet("To design and code a zero-dependency, standalone command-line application in C11 featuring an intuitive menu system for selecting and executing individual system software components.", bold_prefix="1. Integrated Menu-Driven Architecture: ")
    add_bullet("To build a complete lexical scanner capable of classifying C17 source tokens across keywords, identifiers, constants, strings, chars, operators, separators, comments, and preprocessor directives.", bold_prefix="2. Robust Lexical Tokenization: ")
    add_bullet("To implement a dynamic table-driven Symbol Table supporting insertion, searching, memory address assignment starting at 1000, scope tracking, type checking (char=1, int=4, float=4, double=8), deletion, and updating.", bold_prefix="3. Dynamic Symbol Table Management: ")
    add_bullet("To develop a complete Two-Pass Assembler simulation supporting MOVER, MOVEM, ADD, SUB, MULT, DIV, COMP, BC, READ, PRINT, START, END, DC, and DS.", bold_prefix="4. Two-Pass Assembly Translation: ")
    add_bullet("To engineer a Macro Processor that constructs MNT and MDT structures, handles multi-argument MACRO/MEND definitions, and expands macro calls with parameter substitution.", bold_prefix="5. Table-Driven Macro Expansion: ")
    add_bullet("To implement a top-down Recursive Descent Parser that evaluates arithmetic expressions against a formal LL(1) grammar ($E \\rightarrow T E'$, $T \\rightarrow + T E' \\mid - T E' \\mid \\varepsilon$, $T \\rightarrow F T'$, $T' \\rightarrow * F T' \\mid / F T' \\mid \\varepsilon$, $F \\rightarrow (E) \\mid id \\mid num$).", bold_prefix="6. Formal Syntax Parsing: ")
    add_bullet("To create an Intermediate Code Generator that parses mathematical expressions and generates structured 3-address Quadruple records ($Op, Arg1, Arg2, Target$) with temporary variables $T1, T2$.", bold_prefix="7. Intermediate Quadruple Code Generation: ")
    add_bullet("To simulate Linker-Loader functionality strictly file-based, building an External Symbol Table (EST), performing address relocation, and emitting linked memory maps.", bold_prefix="8. File-Based Linking and Relocation: ")
    add_bullet("To construct a Code Optimizer performing Constant Folding, Constant Propagation, Algebraic Simplification, and Dead Code Elimination.", bold_prefix="9. Multi-Technique Code Optimization: ")

    # 9. SCOPE OF THE PROJECT
    add_h1("9. SCOPE OF THE PROJECT")
    add_p("The Mini System Software Toolkit is explicitly designed as an educational simulation framework targeting undergraduate System Software curricula. Its scope includes:")
    add_bullet("Implementing eight foundational system software components as separate C compilation units.", bold_prefix="Full Core Coverage: ")
    add_bullet("All inputs are supplied via text files (`input/*.txt`, `input/*.asm`), and all outputs are written to persistent text files (`output/*.txt`).", bold_prefix="File-Based CLI Architecture: ")
    add_bullet("The Linker Loader module is strictly file-based only, eliminating interactive prompt ambiguity.", bold_prefix="File-Based Linker Loader: ")
    add_bullet("Demonstrates two-pass assembly translation over a custom 10-mnemonic hypothetical machine model.", bold_prefix="Assembly Translation Scope: ")

    doc.add_page_break()

    # 10. SYSTEM REQUIREMENTS
    add_h1("10. SYSTEM REQUIREMENTS")
    add_p("The Mini System Software Toolkit is designed to be lightweight, platform-portable, and zero-dependency. It compiles cleanly on any host machine equipped with an ISO C11 compliant C compiler.")

    add_h2("10.1 Hardware Requirements")
    add_bullet("Intel Core i3 / AMD Ryzen 3 or higher (Compatible with any modern x86_64 or ARM processor).", bold_prefix="Processor: ")
    add_bullet("512 MB minimum (1 GB recommended). Toolkit memory footprint is under 15 MB during execution.", bold_prefix="System RAM: ")
    add_bullet("50 MB available disk space for source code, build binaries, input files, and output artifacts.", bold_prefix="Hard Disk Space: ")
    add_bullet("Standard 80x24 character console terminal (PowerShell, Command Prompt, or Linux Terminal).", bold_prefix="Display/Console: ")

    add_h2("10.2 Software Requirements")
    add_bullet("Microsoft Windows 10 / 11 (x64) or POSIX-compliant Linux / macOS environment.", bold_prefix="Operating System: ")
    add_bullet("ISO C11 Standard (C11 flag: -std=c11). Complies cleanly with GCC 7.0+ or Clang 6.0+.", bold_prefix="C Language Standard: ")
    add_bullet("GNU Compiler Collection (GCC) 9.0 or higher (e.g., MinGW-w64 on Windows).", bold_prefix="Compiler Toolchain: ")
    add_bullet("Windows PowerShell 5.1+ or GNU Make (mingw32-make) for automated project compilation.", bold_prefix="Command Shell & Build Tools: ")
    add_bullet("Standard ASCII / UTF-8 plain text editors (VS Code, Notepad++, VIM) for inspecting text artifacts.", bold_prefix="File Processing Tooling: ")

    # 11. SYSTEM ARCHITECTURE
    add_h1("11. SYSTEM ARCHITECTURE")
    add_p("The Mini System Software Toolkit is structured around a decoupled, modular software architecture. At the top level, `main.c` executes an interactive menu loop that prompts the user to select one of the eight core system software modules. Each module operates independently as a self-contained C compilation unit, accepting file-based input from the `input/` folder, processing the data in memory, and persisting detailed execution logs to the `output/` folder.")

    if os.path.exists('D:\\SS\\Mini-System-Software-Toolkit\\docs\\diagrams\\Diagram images.png'):
        p_img = doc.add_paragraph()
        p_img.alignment = WD_ALIGN_PARAGRAPH.CENTER
        doc.add_picture('D:\\SS\\Mini-System-Software-Toolkit\\docs\\diagrams\\Diagram images.png', width=Inches(5.8))
        p_cap = doc.add_paragraph()
        p_cap.alignment = WD_ALIGN_PARAGRAPH.CENTER
        r_cap = p_cap.add_run("Figure 11.1: Overall Architectural Block Diagram of Mini System Software Toolkit")
        r_cap.bold = True
        r_cap.font.size = Pt(9.5)
        r_cap.font.color.rgb = COLOR_SECONDARY

    add_placeholder_box("Figure 13.1: Main Toolkit Menu Execution Output", "The interactive console menu prompts the user to select any of the 8 modules or exit the application cleanly.")

    doc.add_page_break()

    # 12. PROJECT STRUCTURE
    add_h1("12. PROJECT STRUCTURE")
    add_p("The workspace is organized into a clean directory layout that separates C source code, compiled binaries, documentation, input test files, and output generated artifacts.")

    code_struct = """Mini-System-Software-Toolkit/
├── bin/
│   └── toolkit.exe
├── docs/
│   ├── diagrams/
│   ├── flowcharts/
│   └── screenshots/
├── input/
│   ├── assembler_input.asm
│   ├── lexical_input.txt
│   ├── linker_input.txt
│   ├── macro_input.asm
│   ├── optimizer_input.txt
│   ├── parser_input.txt
│   ├── quadruple_input.txt
│   └── symbol_table_input.txt
├── output/
│   ├── assembler_symbol_table.txt
│   ├── intermediate_code.txt
│   ├── linker_output.txt
│   ├── macro_output.txt
│   ├── object_code.txt
│   ├── optimizer_output.txt
│   ├── parser_output.txt
│   ├── quadruple_output.txt
│   ├── symbol_table.txt
│   └── tokens.txt
├── src/
│   ├── code_optimizer/
│   │   ├── optimizer.c
│   │   └── optimizer.h
│   ├── common/
│   │   ├── common.c
│   │   └── common.h
│   ├── lexical_analyzer/
│   │   ├── lexical_analyzer.c
│   │   └── lexical_analyzer.h
│   ├── linker_loader/
│   │   ├── linker_loader.c
│   │   └── linker_loader.h
│   ├── macro_processor/
│   │   ├── macro_processor.c
│   │   └── macro_processor.h
│   ├── quadruple_generator/
│   │   ├── quadruple.c
│   │   └── quadruple.h
│   ├── recursive_descent_parser/
│   │   ├── parser.c
│   │   └── parser.h
│   ├── symbol_table/
│   │   ├── symbol_table.c
│   │   └── symbol_table.h
│   ├── two_pass_assembler/
│   │   ├── assembler.c
│   │   ├── assembler.h
│   │   ├── pass1.c
│   │   └── pass2.c
│   └── main.c
└── Makefile"""
    add_code_snippet(code_struct, "Table 12.1: Complete Project Folder and Subdirectory Layout")

    # 13. DETAILED MODULE IMPLEMENTATION
    add_h1("13. DETAILED MODULE IMPLEMENTATION")

    add_h2("13.1 Lexical Analyzer")
    add_p("File Input: `input/lexical_input.txt` | Output: `output/tokens.txt`\n"
          "The Lexical Analyzer reads source code and categorizes tokens into keywords, identifiers, integer/float constants, string literals, character constants, operators, separators, comments, and preprocessor directives.")
    snippet_lex = """/* C17 Keyword Lookup & Lexeme Formatting in src/lexical_analyzer/lexical_analyzer.c */
static const char *C17_KEYWORDS[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "inline", "int", "long", "register", "restrict", "return", "short",
    "signed", "sizeof", "static", "struct", "switch", "typedef", "union",
    "unsigned", "void", "volatile", "while"
};
int is_keyword(const char *word) {
    if (!word || *word == '\\0') return 0;
    for (size_t i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(word, C17_KEYWORDS[i]) == 0) return 1;
    }
    return 0;
}"""
    add_code_snippet(snippet_lex, "Code Snippet 13.1: Keyword Scanning Logic in src/lexical_analyzer/lexical_analyzer.c")
    add_placeholder_box("Figure 13.2: Lexical Analyzer Execution and Generated Token Output", "The scanner reads input/lexical_input.txt and persists classified token entries to output/tokens.txt.")

    add_h2("13.2 Symbol Table")
    add_p("File Input: `input/symbol_table_input.txt` | Output: `output/symbol_table.txt`\n"
          "Manages symbols with attributes: Symbol Name, Data Type, Scope, Address, Size. Addresses start automatically from 1000. Data sizes: char=1, int=4, float=4, double=8. Supports Insert, Search, Delete, Update, and Display operations.")
    snippet_sym = """/* Symbol Insertion and Address Allocation in src/symbol_table/symbol_table.c */
int insert_symbol(SymbolTable *st, const char *name, const char *data_type, const char *scope, int address, int size) {
    if (!st || !name || *name == '\\0') return 0;
    if (st->count >= MAX_SYMBOLS) return 0;
    if (search_symbol(st, name) != -1) return 0;
    Symbol *s = &st->symbols[st->count];
    strncpy(s->name, name, MAX_NAME_LEN - 1);
    strncpy(s->data_type, data_type ? data_type : "int", MAX_TYPE_LEN - 1);
    strncpy(s->scope, scope ? scope : "global", MAX_SCOPE_LEN - 1);
    s->size = (size > 0) ? size : get_datatype_size(s->data_type);
    s->address = (address >= 0) ? address : st->next_address;
    st->next_address = s->address + s->size;
    st->count++;
    return 1;
}"""
    add_code_snippet(snippet_sym, "Code Snippet 13.2: Symbol Table Insertion Logic in src/symbol_table/symbol_table.c")
    add_placeholder_box("Figure 13.3: Symbol Table Operations and Result Output", "Displays active symbol attributes, data sizes, addresses, and scope offsets.")

    doc.add_page_break()

    add_h2("13.3 Two Pass Assembler")
    add_p("File Input: `input/assembler_input.asm` | Outputs: `output/assembler_symbol_table.txt`, `output/intermediate_code.txt`, `output/object_code.txt`\n"
          "Implements Pass 1 (LC tracking & Symbol/IC generation) and Pass 2 (Object machine code resolution). Supports mnemonics: MOVER, MOVEM, ADD, SUB, MULT, DIV, COMP, BC, READ, PRINT, START, END, DC, DS.")

    op_table = doc.add_table(rows=11, cols=4)
    op_table.rows[0].cells[0].paragraphs[0].add_run("Mnemonic").bold = True
    op_table.rows[0].cells[1].paragraphs[0].add_run("Class").bold = True
    op_table.rows[0].cells[2].paragraphs[0].add_run("Opcode").bold = True
    op_table.rows[0].cells[3].paragraphs[0].add_run("Description / Operation").bold = True

    op_data = [
        ("STOP", "IS", "00", "Halts execution"),
        ("ADD", "IS", "01", "Reg <- Reg + Memory Operand"),
        ("SUB", "IS", "02", "Reg <- Reg - Memory Operand"),
        ("MULT", "IS", "03", "Reg <- Reg * Memory Operand"),
        ("MOVER", "IS", "04", "Register <- Memory Operand"),
        ("MOVEM", "IS", "05", "Memory <- Register Operand"),
        ("DS", "DL", "01", "Declare Storage space (reserve words)"),
        ("DC", "DL", "02", "Declare Constant value"),
        ("START", "AD", "01", "Start assembly at specified address"),
        ("END", "AD", "02", "End of assembly program source")
    ]
    for r_i, (m, c, o, d) in enumerate(op_data, start=1):
        op_table.rows[r_i].cells[0].paragraphs[0].add_run(m)
        op_table.rows[r_i].cells[1].paragraphs[0].add_run(c)
        op_table.rows[r_i].cells[2].paragraphs[0].add_run(o)
        op_table.rows[r_i].cells[3].paragraphs[0].add_run(d)
    style_table(op_table)

    snippet_asm = """/* Pass 2 Machine Code Generation in src/two_pass_assembler/pass2.c */
int execute_pass2(void) {
    if (!g_pass1_done) return 0;
    for (int i = 0; i < g_inter_code.count; i++) {
        const IntermediateEntry *ie = &g_inter_code.entries[i];
        if (strcmp(ie->statement_type, "AD") == 0) continue;
        ObjectCodeEntry *oe = &g_obj_code.entries[g_obj_code.count++];
        oe->address = ie->address;
        if (strcmp(ie->statement_type, "IS") == 0) {
            oe->opcode = ie->opcode;
            oe->reg_code = ie->reg_code;
            int sym_idx = find_asm_symbol(&g_asm_symtab, ie->operand_name);
            if (sym_idx != -1) oe->operand_address = g_asm_symtab.symbols[sym_idx].address;
        }
    }
    return 1;
}"""
    add_code_snippet(snippet_asm, "Code Snippet 13.3: Pass 2 Machine Code Resolution in src/two_pass_assembler/pass2.c")
    add_placeholder_box("Figure 13.4a: Two Pass Assembler Assembly Source Input", "Shows raw assembly source file containing instructions and directives.")
    add_placeholder_box("Figure 13.4b: Two Pass Assembler Intermediate Code, Symbol Table, and Object Code Output", "Displays generated Intermediate Code, Assembler Symbol Table, and Object Code.")

    add_h2("13.4 Macro Processor")
    add_p("File Input: `input/macro_input.asm` | Output: `output/macro_output.txt`\n"
          "Processes MACRO and MEND directives, builds Macro Name Table (MNT) and Macro Definition Table (MDT), and performs formal-to-actual parameter substitution during macro expansion.")
    snippet_mac = """/* Parameter Substitution in src/macro_processor/macro_processor.c */
static void replace_parameter(const char *template_str, const char *formal, const char *actual, char *result, size_t result_size) {
    result[0] = '\\0';
    const char *pos = template_str;
    const char *found;
    size_t formal_len = strlen(formal);
    while ((found = strstr(pos, formal)) != NULL) {
        strncat(result, pos, found - pos);
        strcat(result, actual);
        pos = found + formal_len;
    }
    strcat(result, pos);
}"""
    add_code_snippet(snippet_mac, "Code Snippet 13.4: Parameter Substitution in src/macro_processor/macro_processor.c")
    add_placeholder_box("Figure 13.5: Macro Processor Definition and Expanded Output", "Shows MNT/MDT records and final expanded assembly output.")

    doc.add_page_break()

    add_h2("13.5 Recursive Descent Parser")
    add_p("File Input: `input/parser_input.txt` | Output: `output/parser_output.txt`\n"
          "Validates arithmetic expressions top-down against formal LL(1) grammar:\n"
          "E -> T E'\nE' -> + T E' | - T E' | epsilon\nT -> F T'\nT' -> * F T' | / F T' | epsilon\nF -> ( E ) | id | number\n"
          "Supports both file input and direct expression input, demonstrating valid and invalid expression handling.")
    snippet_par = """/* Recursive Descent Factor Parsing in src/recursive_descent_parser/parser.c */
static int parse_factor(void) {
    skip_whitespace();
    char c = g_input[g_pos];
    if (c == '(') {
        g_pos++;
        if (!parse_expression()) return 0;
        skip_whitespace();
        if (g_input[g_pos] == ')') { g_pos++; return 1; }
        else { set_error("Expected ')'"); return 0; }
    }
    if (isalpha((unsigned char)c) || c == '_') {
        while (isalnum((unsigned char)g_input[g_pos]) || g_input[g_pos] == '_') g_pos++;
        return 1;
    }
    if (isdigit((unsigned char)c)) {
        while (isdigit((unsigned char)g_input[g_pos]) || g_input[g_pos] == '.') g_pos++;
        return 1;
    }
    set_error("Unexpected token in factor");
    return 0;
}"""
    add_code_snippet(snippet_par, "Code Snippet 13.5: Factor Parsing in src/recursive_descent_parser/parser.c")
    add_placeholder_box("Figure 13.6: Recursive Descent Parser Valid and Invalid Expression Result", "Displays syntax parsing results for valid expressions and error reporting for invalid strings.")

    add_h2("13.6 Quadruple Generator")
    add_p("File Input: `input/quadruple_input.txt` | Output: `output/quadruple_output.txt`\n"
          "Translates arithmetic expressions (+, -, *, /, parentheses) into 3-address quadruples respecting operator precedence and allocating temporary variables T1, T2, etc.\n"
          "Example Input: `a + b * c`  =>  Quadruples: `(*, b, c, T1)` and `(+, a, T1, T2)`.")
    snippet_quad = """/* Precedence Evaluation in src/quadruple_generator/quadruple.c */
static int get_precedence(const char *op) {
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) return 2;
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 1;
    return 0;
}"""
    add_code_snippet(snippet_quad, "Code Snippet 13.6: Precedence Check in src/quadruple_generator/quadruple.c")
    add_placeholder_box("Figure 13.7: Quadruple Generator 3-Address Intermediate Code Output", "Displays generated 4-tuple quadruple records for input arithmetic expressions.")

    add_h2("13.7 Linker Loader")
    add_p("File Input: `input/linker_input.txt` | Output: `output/linker_output.txt`\n"
          "CRITICAL REQUIREMENT: This module is FILE-BASED ONLY. Simulates linking and loading across multiple object modules by resolving external symbols, constructing an External Symbol Table (EST), applying relocation offsets, and generating a linked memory map.")
    snippet_link = """/* EST Construction in src/linker_loader/linker_loader.c */
static int build_external_symbol_table(Module *modules, int mod_count, ESTEntry *est, int *est_count) {
    *est_count = 0;
    for (int i = 0; i < mod_count; i++) {
        for (int j = 0; j < modules[i].def_count; j++) {
            ESTEntry *entry = &est[*est_count];
            strncpy(entry->symbol_name, modules[i].defs[j].symbol_name, MAX_NAME_LEN - 1);
            entry->abs_address = modules[i].load_address + modules[i].defs[j].relative_address;
            strncpy(entry->module_name, modules[i].name, MAX_NAME_LEN - 1);
            (*est_count)++;
        }
    }
    return 1;
}"""
    add_code_snippet(snippet_link, "Code Snippet 13.7: EST Construction in src/linker_loader/linker_loader.c")
    add_placeholder_box("Figure 13.8: Linker Loader External Symbol Table and Linked Memory Map Output", "Displays generated External Symbol Table (EST) and absolute relocated memory layout.")

    add_h2("13.8 Code Optimizer")
    add_p("File Input: `input/optimizer_input.txt` | Output: `output/optimizer_output.txt`\n"
          "Applies four optimization passes:\n"
          "1. Constant Folding: `a = 10 + 20` -> `a = 30`\n"
          "2. Constant Propagation: Replaces variable references with known constants\n"
          "3. Algebraic Simplification: `x = y + 0` -> `x = y`, `z = x * 1` -> `z = x`, `d = y * 0` -> `d = 0`\n"
          "4. Dead Code Elimination: Removes unreferenced assignments.")
    snippet_opt = """/* Algebraic Simplification in src/code_optimizer/optimizer.c */
static int optimize_algebraic(char *op, char *arg1, char *arg2, char *result_val) {
    if (strcmp(op, "+") == 0 && strcmp(arg2, "0") == 0) { strcpy(result_val, arg1); return 1; }
    if (strcmp(op, "*") == 0 && strcmp(arg2, "1") == 0) { strcpy(result_val, arg1); return 1; }
    if (strcmp(op, "*") == 0 && strcmp(arg2, "0") == 0) { strcpy(result_val, "0"); return 1; }
    return 0;
}"""
    add_code_snippet(snippet_opt, "Code Snippet 13.8: Algebraic Simplification in src/code_optimizer/optimizer.c")
    add_placeholder_box("Figure 13.9: Code Optimizer Input and Optimized Code Output", "Displays original intermediate statements alongside optimized output representations.")

    doc.add_page_break()

    # 14. ALGORITHMS / WORKING METHODOLOGY
    add_h1("14. ALGORITHMS / WORKING METHODOLOGY")
    add_p("The toolkit operates through a structured command-line menu loop implemented in `main.c`. Selecting an option invokes the respective module handler, which opens the target input file, parses tokens or records into memory structures, performs transformation algorithms, and writes formatted execution logs to the output folder.")

    if os.path.exists('D:\\SS\\Mini-System-Software-Toolkit\\docs\\flowcharts\\all flowcharts.png'):
        p_img2 = doc.add_paragraph()
        p_img2.alignment = WD_ALIGN_PARAGRAPH.CENTER
        doc.add_picture('D:\\SS\\Mini-System-Software-Toolkit\\docs\\flowcharts\\all flowcharts.png', width=Inches(5.8))
        p_cap2 = doc.add_paragraph()
        p_cap2.alignment = WD_ALIGN_PARAGRAPH.CENTER
        r_cap2 = p_cap2.add_run("Figure 14.1: Master Control Flowchart Suite for All Toolkit Modules")
        r_cap2.bold = True
        r_cap2.font.size = Pt(9.5)
        r_cap2.font.color.rgb = COLOR_SECONDARY

    # 15. INPUT AND OUTPUT DESCRIPTION
    add_h1("15. INPUT AND OUTPUT DESCRIPTION")
    add_p("The table below specifies the exact input and output file mapping for all eight modules:")

    io_table = doc.add_table(rows=9, cols=3)
    io_table.rows[0].cells[0].paragraphs[0].add_run("Module").bold = True
    io_table.rows[0].cells[1].paragraphs[0].add_run("Input").bold = True
    io_table.rows[0].cells[2].paragraphs[0].add_run("Output").bold = True

    io_data = [
        ("Lexical Analyzer", "lexical_input.txt", "tokens.txt"),
        ("Symbol Table", "symbol_table_input.txt", "symbol_table.txt"),
        ("Two Pass Assembler", "assembler_input.asm", "intermediate_code.txt, object_code.txt"),
        ("Macro Processor", "macro_input.asm", "macro_output.txt"),
        ("Recursive Descent Parser", "parser_input.txt", "parser_output.txt"),
        ("Quadruple Generator", "quadruple_input.txt", "quadruple_output.txt"),
        ("Linker Loader", "linker_input.txt", "linker_output.txt"),
        ("Code Optimizer", "optimizer_input.txt", "optimizer_output.txt")
    ]
    for r_idx, (m, i, o) in enumerate(io_data, start=1):
        io_table.rows[r_idx].cells[0].paragraphs[0].add_run(m)
        io_table.rows[r_idx].cells[1].paragraphs[0].add_run(i)
        io_table.rows[r_idx].cells[2].paragraphs[0].add_run(o)
    style_table(io_table)

    # 16. TESTING AND RESULTS
    add_h1("16. TESTING AND RESULTS")
    add_p("The system was validated against valid input files and error/edge cases supported by the C11 implementation:")

    test_table = doc.add_table(rows=10, cols=6)
    test_table.rows[0].cells[0].paragraphs[0].add_run("Test Case").bold = True
    test_table.rows[0].cells[1].paragraphs[0].add_run("Module").bold = True
    test_table.rows[0].cells[2].paragraphs[0].add_run("Input / Condition").bold = True
    test_table.rows[0].cells[3].paragraphs[0].add_run("Expected Result").bold = True
    test_table.rows[0].cells[4].paragraphs[0].add_run("Actual Result").bold = True
    test_table.rows[0].cells[5].paragraphs[0].add_run("Status").bold = True

    test_data = [
        ("TC-01", "Lexical Analyzer", "int count = 10;", "Categorize 5 tokens", "Tokens logged to tokens.txt", "PASS"),
        ("TC-02", "Symbol Table", "Insert duplicate 'x'", "Emit duplicate warning", "Warning printed, table preserved", "PASS"),
        ("TC-03", "Two Pass Assembler", "Valid ASM program", "Generate IC & Object Code", "intermediate_code & object_code emitted", "PASS"),
        ("TC-04", "Two Pass Assembler", "Undefined label 'L2'", "Aborts Pass 2 with error", "Error message displayed, Pass 2 stopped", "PASS"),
        ("TC-05", "Macro Processor", "Macro with 2 args", "Expand body & substitute args", "macro_output.txt updated correctly", "PASS"),
        ("TC-06", "Parser", "Expression: a + (b * c)", "Valid expression result", "Parse Successful printed", "PASS"),
        ("TC-07", "Parser", "Invalid: a + * b", "Detect syntax error", "Syntax Error: Expected factor logged", "PASS"),
        ("TC-08", "Linker Loader", "2 Object Modules", "Build EST & Linked Map", "linker_output.txt generated", "PASS"),
        ("TC-09", "Code Optimizer", "a = 10 + 20; x = y * 1", "Fold to 30; Simplify to x = y", "optimizer_output.txt optimized", "PASS")
    ]

    for r_idx, (tc, m, i, e, a, s) in enumerate(test_data, start=1):
        test_table.rows[r_idx].cells[0].paragraphs[0].add_run(tc)
        test_table.rows[r_idx].cells[1].paragraphs[0].add_run(m)
        test_table.rows[r_idx].cells[2].paragraphs[0].add_run(i)
        test_table.rows[r_idx].cells[3].paragraphs[0].add_run(e)
        test_table.rows[r_idx].cells[4].paragraphs[0].add_run(a)
        test_table.rows[r_idx].cells[5].paragraphs[0].add_run(s)
    style_table(test_table)

    doc.add_page_break()

    # 17-21. ADVANTAGES, LIMITATIONS, FUTURE SCOPE, LEARNING OUTCOMES, CONCLUSION
    add_h1("17. ADVANTAGES")
    add_bullet("Combines 8 core system software components into a single menu-driven executable.", bold_prefix="1. Integrated Architecture: ")
    add_bullet("Written in ISO C11 with zero external third-party dependencies.", bold_prefix="2. High Portability: ")
    add_bullet("Transparent file-based inputs and output logs allow full visibility into translation passes.", bold_prefix="3. Clear Traceability: ")
    add_bullet("Decoupled module design makes it easy for students to isolate and study specific algorithms.", bold_prefix="4. Modular Design: ")

    add_h1("18. LIMITATIONS")
    add_bullet("Designed primarily as an academic educational simulation tool.", bold_prefix="1. Academic Scope: ")
    add_bullet("Uses fixed static buffer limits (e.g., MAX_SYMBOLS=100) instead of dynamic memory allocation.", bold_prefix="2. Static Buffer Allocation: ")
    add_bullet("Assembler targets a simplified 10-mnemonic instruction set.", bold_prefix="3. Custom Instruction Model: ")
    add_bullet("Operates via text terminal menus rather than a graphical user interface.", bold_prefix="4. Terminal Interface: ")

    add_h1("19. FUTURE SCOPE")
    add_bullet("Develop a web-based visual simulation interface using WebAssembly.", bold_prefix="1. Web Dashboard: ")
    add_bullet("Expand assembler support to real x86_64 or RISC-V instruction sets.", bold_prefix="2. RISC-V ISA Extension: ")
    add_bullet("Implement control-flow graph (CFG) analysis and loop-invariant code motion.", bold_prefix="3. CFG Optimization: ")

    add_h1("20. LEARNING OUTCOMES")
    add_bullet("Gained practical proficiency in lexical scanning and symbol table management.", bold_prefix="1. Lexical & Symbol Management: ")
    add_bullet("Mastered two-pass assembly translation and macro expansion mechanics.", bold_prefix="2. Assemblers & Macro Processing: ")
    add_bullet("Implemented top-down predictive parsing and 3-address quadruple generation.", bold_prefix="3. Syntax Parsing & Intermediate Code: ")
    add_bullet("Understood external symbol resolution, relocation, and machine-independent optimization.", bold_prefix="4. Linking, Loading & Optimization: ")

    add_h1("21. CONCLUSION")
    add_p("The Mini System Software Toolkit successfully bridges theoretical computer engineering education with practical C11 software engineering. By implementing eight core system software components—Lexical Analyzer, Symbol Table, Two-Pass Assembler, Macro Processor, Recursive Descent Parser, Quadruple Generator, Linker Loader, and Code Optimizer—within a clean, menu-driven executable architecture, the project provides an invaluable educational tool.")

    add_h1("22. REFERENCES")
    add_bullet("D. M. Dhamdhere, \"System Programming and Operating Systems\", 2nd Revised Edition, Tata McGraw-Hill Education, 2011.")
    add_bullet("A. V. Aho, M. S. Lam, R. Sethi, and J. D. Ullman, \"Compilers: Principles, Techniques, and Tools\" (Dragon Book), 2nd Edition, Pearson / Addison-Wesley, 2006.")
    add_bullet("L. L. Beck, \"System Software: An Introduction to Systems Programming\", 3rd Edition, Pearson Education, 2002.")
    add_bullet("ISO/IEC 9899:2011, \"Information technology — Programming languages — C\" (C11 Standard Specification).")
    add_bullet("GNU Compiler Collection (GCC) Documentation, \"GCC Command Options and C Standards Compliance\", Free Software Foundation, 2024.")

    # 23. APPENDIX
    add_h1("23. APPENDIX")
    add_h2("Appendix A – Project Repository")
    add_p("GitHub Repository:\n[PASTE GITHUB REPOSITORY LINK HERE]", bold_prefix="Source Code Repository Placeholder: ")
    add_p("The repository contains:\n"
          "- Complete C11 source code (`src/`)\n"
          "- Input files (`input/`)\n"
          "- Output files (`output/`)\n"
          "- Makefile\n"
          "- README.md\n"
          "- Documentation (`docs/`)\n"
          "- Diagrams (`docs/diagrams/`)\n"
          "- Flowcharts (`docs/flowcharts/`)")

    output_dir = 'D:\\SS\\Mini-System-Software-Toolkit\\docs'
    os.makedirs(output_dir, exist_ok=True)
    target_path = os.path.join(output_dir, 'Mini_System_Software_Toolkit_PBL_Report.docx')

    try:
        doc.save(target_path)
        print(f"PBL Report successfully saved at: {target_path}")
    except PermissionError:
        alt_path = os.path.join(output_dir, 'Mini_System_Software_Toolkit_PBL_Report_Updated.docx')
        doc.save(alt_path)
        print(f"Target file was locked by Word. PBL Report saved to alternative path: {alt_path}")

if __name__ == '__main__':
    create_pbl_report()
